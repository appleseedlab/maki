#include "Cpp2CASTConsumer.hh"
#include "DeclStmtTypeLoc.hh"
#include "ExpansionMatchHandler.hh"
#include "ExpansionMatcher.hh"
#include "StmtCollectorMatchHandler.hh"

#include "clang/Lex/Lexer.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

#include <algorithm>
#include <functional>

#include "assert.h"

// TODO:    Remember to check if we should be ignoring implicit casts
//          and if we should be using TK_IgnoreUnlessSpelledInSource
//          and ignoringImplicit

namespace cpp2c
{
    DeclStmtTypeLoc *firstASTRootExpandsToAlignWithRange(
        clang::SourceManager &SM,
        std::vector<DeclStmtTypeLoc> &ASTRoots,
        clang::SourceRange Range)
    {

        // TODO: Maybe instead we should align the expansion with the root
        // that has the most inclusive range?
        for (auto &&R : ASTRoots)
            if (Range == SM.getExpansionRange(R.getSourceRange()).getAsRange())
                return &R;
        return nullptr;
    }

    Cpp2CASTConsumer::Cpp2CASTConsumer(clang::CompilerInstance &CI)
    {
        clang::Preprocessor &PP = CI.getPreprocessor();
        clang::ASTContext &Ctx = CI.getASTContext();

        MF = new cpp2c::MacroForest(PP, Ctx);

        PP.addPPCallbacks(std::unique_ptr<cpp2c::MacroForest>(MF));
    }

    bool isInTree(
        const clang::Stmt *ST,
        std::function<bool(const clang::Stmt *)> pred)
    {
        if (pred(ST))
            return true;
        for (auto child : ST->children())
            if (isInTree(child, pred))
                return true;
        return false;
    }

    template <typename T>
    inline std::function<bool(const clang::Stmt *)> stmtIsA()
    {
        return [](const clang::Stmt *ST)
        { return clang::isa<T>(ST); };
    }

    inline std::function<bool(const clang::Stmt *)>
    stmtIsBinOp(clang::BinaryOperator::Opcode OC)
    {
        return [OC](const clang::Stmt *ST)
        {
            if (auto BO = clang::dyn_cast<clang::BinaryOperator>(ST))
                return BO->getOpcode() == OC;
            return false;
        };
    }

    inline std::function<bool(MacroExpansionArgument)>
    locInArg(clang::SourceLocation L)
    {
        return [L](MacroExpansionArgument Arg)
        {
            if (Arg.Tokens.empty())
                return false;
            clang::SourceRange ArgTokRange(
                Arg.Tokens.front().getLocation(),
                Arg.Tokens.back().getEndLoc());
            return ArgTokRange.fullyContains(L);
        };
    }

    template <typename T>
    std::vector<const clang::Stmt *>
    matchAndCollectStmtsIn(
        clang::ASTContext &Ctx,
        T *Node,
        const clang::ast_matchers::internal::BindableMatcher<clang::Stmt> m)
    {
        using namespace clang::ast_matchers;
        MatchFinder Finder;
        StmtCollectorMatchHandler Handler;

        // We have to add two matchers because by default,
        // clang appears to not recursively match on AST nodes when given
        // a specific node to look for matches in

        // Matcher for subtrees
        auto SubtreeMatcher = m;
        // Matcher for recursively applying the subtree matcher to the root node
        auto RootMatcher = stmt(forEachDescendant(SubtreeMatcher));

        // According to Dietrich, the order in which we apply
        // these matchers is important, but I'm not sure why.
        Finder.addMatcher(SubtreeMatcher, &Handler);
        Finder.addMatcher(RootMatcher, &Handler);
        Finder.match(*Node, Ctx);
        return Handler.Stmts;
    }

    // Checks that a given expansion is hygienic.
    // By hygienic, we mean that it satisfies Clinger and Rees'
    // strong hygiene condition for macros:
    // Local variables in the expansion must have been passed as arguments,
    // and the expansion must not create new declarations that can be
    // accessed outside of the expansion.
    // TODO: Check for new declarations!
    //       Right now we only check for unbound local vars.
    bool isHygienic(
        clang::ASTContext &Ctx,
        cpp2c::MacroExpansionNode *Expansion)
    {
        assert(Expansion->AlignedRoot &&
               Expansion->AlignedRoot->ST &&
               "Expansion must have an aligned Stmt root");
        // 1. Collect all local vars in the expansion
        std::vector<const clang::DeclRefExpr *> LocalVars;
        for (auto ST : matchAndCollectStmtsIn<const clang::Stmt>(
                 Ctx,
                 Expansion->AlignedRoot->ST,
                 stmt(unless(implicitCastExpr()),
                      declRefExpr(to(varDecl(hasLocalStorage())))
                          .bind("root"))))
            if (auto DRE = clang::dyn_cast<clang::DeclRefExpr>(ST))
                LocalVars.push_back(DRE);
            else
                assert(!"Matched a non DeclRefExpr");

        // 2. Check if any local vars were not passed as arguments.
        //    If a local var was spelled inside any of the macro's
        //    arguments, then its hygienic; otherwise, it's not.
        auto &SM = Ctx.getSourceManager();
        for (auto DRE : LocalVars)
        {
            // NOTE: Maybe we should use endLoc here?
            auto L = SM.getSpellingLoc(DRE->getBeginLoc());
            if (!std::any_of(Expansion->Arguments.begin(),
                             Expansion->Arguments.end(),
                             locInArg(L)))
                return false;
        }
        // 3. TODO: Check if the expansion contains a decl that could be
        //          accessed after the macro is expanded.
        //          (i.e., if the expansion contains a decl that is not
        //           inside a compound stmt).
        return true;
    }

    bool isParameterSideEffectFree(
        clang::ASTContext &Ctx,
        MacroExpansionNode *Expansion)
    {
        assert(Expansion->AlignedRoot &&
               Expansion->AlignedRoot->ST &&
               "Expansion must have an aligned Stmt root");
        // 1. Collect all expressions with side-effects in the expansion
        std::vector<const clang::Expr *> SideEffectExprs;
        for (auto ST : matchAndCollectStmtsIn<const clang::Stmt>(
                 Ctx,
                 Expansion->AlignedRoot->ST,
                 stmt(unless(implicitCastExpr()),
                      anyOf(
                          binaryOperator(isAssignmentOperator()).bind("root"),
                          unaryOperator(hasOperatorName("++")).bind("root"),
                          unaryOperator(hasOperatorName("--")).bind("root")))))
            if (auto Ex = clang::dyn_cast<clang::Expr>(ST))
                SideEffectExprs.push_back(Ex);
            else
                assert(!"Matched a non Expr");

        // 2. Check that none of these side-effect exprs came from
        //    a macro argument
        auto &SM = Ctx.getSourceManager();
        return !(std::any_of(SideEffectExprs.begin(),
                             SideEffectExprs.end(),
                             [&SM, &Expansion](const clang::Expr *Ex)
                             {
                                 auto L = SM.getSpellingLoc(Ex->getBeginLoc());
                                 return std::any_of(
                                     Expansion->Arguments.begin(),
                                     Expansion->Arguments.end(),
                                     locInArg(L));
                             }));
    }

    bool isLValueIndependent(
        clang::ASTContext &Ctx,
        MacroExpansionNode *Expansion)
    {
        // 1. Collect & exprs and side-effecting exprs
        std::vector<const clang::Expr *> LValueExprs;
        for (auto ST : matchAndCollectStmtsIn<const clang::Stmt>(
                 Ctx,
                 Expansion->AlignedRoot->ST,
                 stmt(unless(implicitCastExpr()),
                      anyOf(
                          unaryOperator(hasOperatorName("&")).bind("root"),
                          binaryOperator(isAssignmentOperator()).bind("root"),
                          unaryOperator(hasOperatorName("++")).bind("root"),
                          unaryOperator(hasOperatorName("--")).bind("root")))))
            if (auto Ex = clang::dyn_cast<clang::Expr>(ST))
                LValueExprs.push_back(Ex);
            else
                assert(!"Matched a non Expr");

        // 2. Check that either the entire expr came from an argument,
        //    or the expr's operand did not come from an argument
        // TODO: clean this up
        auto &SM = Ctx.getSourceManager();
        for (auto Ex : LValueExprs)
        {
            // Check if the entire expr came from an argument
            auto L = SM.getSpellingLoc(Ex->getBeginLoc());
            if (std::any_of(
                    Expansion->Arguments.begin(),
                    Expansion->Arguments.end(),
                    locInArg(L)))
                continue;

            // If not, the expression may still be safe if the L-value operand
            // does not contain any subtree that came from an argument
            const clang::Expr *Operand = nullptr;
            if (auto AO = clang::dyn_cast<clang::UnaryOperator>(Ex))
                Operand = AO->getSubExpr();
            else if (auto BO = clang::dyn_cast<clang::BinaryOperator>(Ex))
                Operand = BO->getLHS();
            else
                assert(!"Matched a non &, non binary assignment operator");

            for (auto Arg : Expansion->Arguments)
                if (isInTree(
                        Operand,
                        [&SM, &Arg](const clang::Stmt *ST)
                        {
                            auto L = SM.getSpellingLoc(ST->getBeginLoc());
                            return locInArg(L)(Arg);
                        }))
                    return false;
        }
        return true;
    }

    void Cpp2CASTConsumer::HandleTranslationUnit(clang::ASTContext &Ctx)
    {
        auto &SM = Ctx.getSourceManager();
        for (auto TLE : MF->TopLevelExpansions)
        {
            using namespace clang::ast_matchers;

            //// Find potential AST roots of the entire expansion

            // Match stmts
            MATCH_EXPANSION_ROOTS_OF(stmt, TLE);

            // Match decls
            MATCH_EXPANSION_ROOTS_OF(decl, TLE);

            // Match type locs
            MATCH_EXPANSION_ROOTS_OF(typeLoc, TLE);

            TLE->AlignedRoot = firstASTRootExpandsToAlignWithRange(
                SM,
                TLE->ASTRoots,
                TLE->SpellingRange);

            //// Find AST roots aligned with each of the expansion's arguments
            for (auto &&Arg : TLE->Arguments)
            {
                // Match stmts
                MATCH_ARGUMENT_ROOTS_OF(stmt, (&Arg));

                // Match decls
                MATCH_ARGUMENT_ROOTS_OF(decl, (&Arg));

                // Match type locs
                MATCH_ARGUMENT_ROOTS_OF(typeLoc, (&Arg));
            }

            //// Print macro expansion info

            // TLE->dumpASTInfo(llvm::errs(), SM, Ctx.getLangOpts());

            if (!TLE->AlignedRoot)
                llvm::errs() << "Unaligned body,";
            else
            {
                llvm::errs() << "Aligned body,";
                auto D = TLE->AlignedRoot->D;
                auto ST = TLE->AlignedRoot->ST;
                auto TL = TLE->AlignedRoot->TL;

                if (ST)
                    llvm::errs() << "Stmt,";

                if (llvm::isa_and_nonnull<clang::DoStmt>(ST))
                    llvm::errs() << "DoStmt,";
                else
                {
                    if (isInTree(ST, stmtIsA<clang::ContinueStmt>()))
                        llvm::errs() << "ContinueStmt,";
                    if (isInTree(ST, stmtIsA<clang::BreakStmt>()))
                        llvm::errs() << "BreakStmt,";
                }

                if (isInTree(ST, stmtIsA<clang::ReturnStmt>()))
                    llvm::errs() << "ReturnStmt,";
                if (isInTree(ST, stmtIsA<clang::GotoStmt>()))
                    llvm::errs() << "GotoStmt,";

                if (llvm::isa_and_nonnull<clang::CharacterLiteral>(ST))
                    llvm::errs() << "CharacterLiteral,";
                if (llvm::isa_and_nonnull<clang::IntegerLiteral>(ST))
                    llvm::errs() << "IntegerLiteral,";
                if (llvm::isa_and_nonnull<clang::FloatingLiteral>(ST))
                    llvm::errs() << "FloatingLiteral,";
                if (llvm::isa_and_nonnull<clang::FixedPointLiteral>(ST))
                    llvm::errs() << "FixedPointLiteral,";
                if (llvm::isa_and_nonnull<clang::ImaginaryLiteral>(ST))
                    llvm::errs() << "ImaginaryLiteral,";

                if (llvm::isa_and_nonnull<clang::StringLiteral>(ST))
                    llvm::errs() << "StringLiteral,";
                if (llvm::isa_and_nonnull<clang::CompoundLiteralExpr>(ST))
                    llvm::errs() << "CompoundLiteralExpr,";

                if (isInTree(ST, stmtIsA<clang::ConditionalOperator>()))
                    llvm::errs() << "ConditionalOperator,";
                if (isInTree(ST,
                             stmtIsBinOp(
                                 clang::BinaryOperator::Opcode::BO_LAnd)))
                    llvm::errs() << "BinaryOperator::Opcode::BO_LAnd,";
                if (isInTree(ST,
                             stmtIsBinOp(
                                 clang::BinaryOperator::Opcode::BO_LOr)))
                    llvm::errs() << "BinaryOperator::Opcode::BO_LOr,";

                if (D)
                    llvm::errs() << "Decl,";

                if (TL)
                    llvm::errs() << "TypeLoc,";
            }

            // Check that the number of AST nodes aligned with each argument
            // equals the number of times that argument was expanded
            if (std::all_of(TLE->Arguments.begin(),
                            TLE->Arguments.end(),
                            [](MacroExpansionArgument Arg)
                            { return Arg.AlignedRoots.size() ==
                                     Arg.numberOfTimesExpanded; }))
                llvm::errs() << "Aligned arguments,";
            else
                llvm::errs() << "Unaligned argument,";

            // Check for semantic properties of interface-equivalence
            // TODO: Check for these properties in decls as well?

            if (TLE->AlignedRoot &&
                TLE->AlignedRoot->ST)
            {
                if (isHygienic(Ctx, TLE))
                    llvm::errs() << "Hygienic,";
                else
                    llvm::errs() << "Unhygienic,";
                if (isParameterSideEffectFree(Ctx, TLE))
                    llvm::errs() << "Parameter side-effect free,";
                else
                    llvm::errs() << "Parameter side-effects,";
                if (isLValueIndependent(Ctx, TLE))
                    llvm::errs() << "L-value independent,";
                else
                    llvm::errs() << "L-value dependent,";
            }
            else
            {
                llvm::errs() << "Cannot check for hygiene,";
                llvm::errs() << "Cannot check for parameter side-effects,";
                llvm::errs() << "Cannot check for L-value independence,";
            }

            llvm::errs() << "\n";
        }
    }
} // namespace cpp2c
