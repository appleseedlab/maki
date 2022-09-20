#include "Cpp2CASTConsumer.hh"
#include "ASTUtils.hh"
#include "DeclStmtTypeLoc.hh"
#include "ExpansionMatchHandler.hh"
#include "ExpansionMatcher.hh"
#include "Properties.hh"

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
// NOTE:    We can't use TK_IgnoreUnlessSpelledInSource because it ignores
//          paren exprs

namespace cpp2c
{
    Cpp2CASTConsumer::Cpp2CASTConsumer(clang::CompilerInstance &CI)
    {
        clang::Preprocessor &PP = CI.getPreprocessor();
        clang::ASTContext &Ctx = CI.getASTContext();

        MF = new cpp2c::MacroForest(PP, Ctx);

        PP.addPPCallbacks(std::unique_ptr<cpp2c::MacroForest>(MF));
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

    void Cpp2CASTConsumer::HandleTranslationUnit(clang::ASTContext &Ctx)
    {
        for (auto TLE : MF->TopLevelExpansions)
        {
            using namespace clang::ast_matchers;

            //// Find potential AST roots of the entire expansion

            // Match stmts
            if (!(TLE)->DefinitionTokens.empty())
            {
                MatchFinder Finder;
                ExpansionMatchHandler Handler;
                auto Matcher = stmt(unless(anyOf(implicitCastExpr(),
                                                 implicitValueInitExpr())),
                                    alignsWithExpansion(&Ctx, TLE))
                                   .bind("root");
                Finder.addMatcher(Matcher, &Handler);
                Finder.matchAST(Ctx);
                for (auto M : Handler.Matches)
                    TLE->ASTRoots.push_back(M);
            }

            // Match decls
            if (!(TLE->DefinitionTokens.empty()))
            {
                MatchFinder Finder;
                ExpansionMatchHandler Handler;
                auto Matcher = decl(alignsWithExpansion(&Ctx, TLE))
                                   .bind("root");
                Finder.addMatcher(Matcher, &Handler);
                Finder.matchAST(Ctx);
                for (auto M : Handler.Matches)
                    TLE->ASTRoots.push_back(M);
            }

            // Match type locs
            if (!((TLE)->DefinitionTokens.empty()))
            {
                MatchFinder Finder;
                ExpansionMatchHandler Handler;
                auto Matcher = typeLoc(alignsWithExpansion(&Ctx, (TLE)))
                                   .bind("root");
                Finder.addMatcher(Matcher, &Handler);
                Finder.matchAST(Ctx);
                for (auto M : Handler.Matches)
                    TLE->ASTRoots.push_back(M);
            }

            // If the expansion only aligns with one node, then set this
            // as its aligned root
            if (TLE->ASTRoots.size() == 1)
                TLE->AlignedRoot = &(TLE->ASTRoots.front());

            //// Find AST roots aligned with each of the expansion's arguments

            for (auto &&Arg : TLE->Arguments)
            {
                // Match stmts
                if (!Arg.Tokens.empty())
                {
                    MatchFinder Finder;
                    ExpansionMatchHandler Handler;
                    auto Matcher = stmt(unless(anyOf(implicitCastExpr(),
                                                     implicitValueInitExpr())),
                                        isSpelledFromTokens(&Ctx, Arg.Tokens))
                                       .bind("root");
                    Finder.addMatcher(Matcher, &Handler);
                    Finder.matchAST(Ctx);
                    for (auto M : Handler.Matches)
                        Arg.AlignedRoots.push_back(M);
                }

                // Match decls
                if (!(Arg.Tokens.empty()))
                {
                    MatchFinder Finder;
                    ExpansionMatchHandler Handler;
                    auto Matcher = decl(isSpelledFromTokens(&Ctx, Arg.Tokens))
                                       .bind("root");
                    Finder.addMatcher(Matcher, &Handler);
                    Finder.matchAST(Ctx);
                    for (auto M : Handler.Matches)
                        Arg.AlignedRoots.push_back(M);
                }

                // Match type locs
                if (!(Arg.Tokens.empty()))
                {
                    MatchFinder Finder;
                    ExpansionMatchHandler Handler;
                    auto Matcher =
                        typeLoc(isSpelledFromTokens(&Ctx, Arg.Tokens))
                            .bind("root");
                    Finder.addMatcher(Matcher, &Handler);
                    Finder.matchAST(Ctx);
                    for (auto M : Handler.Matches)
                        Arg.AlignedRoots.push_back(M);
                }
            }

            //// Print macro expansion info

            // TLE->dumpASTInfo(llvm::errs(),
            //                  Ctx.getSourceManager(), Ctx.getLangOpts());

            if (TLE->ASTRoots.size() == 0)
                llvm::errs() << "No aligned body,";
            else if (TLE->ASTRoots.size() > 1)
                llvm::errs() << "Multiple aligned bodies,";
            else
            {
                llvm::errs() << "Single aligned body,";
                auto D = TLE->AlignedRoot->D;
                auto ST = TLE->AlignedRoot->ST;
                auto TL = TLE->AlignedRoot->TL;

                if (ST)
                {

                    llvm::errs() << "Stmt,";

                    if (llvm::isa<clang::DoStmt>(ST))
                        llvm::errs() << "DoStmt,";
                    if (llvm::isa<clang::ContinueStmt>(ST))
                        llvm::errs() << "ContinueStmt,";
                    if (llvm::isa<clang::BreakStmt>(ST))
                        llvm::errs() << "BreakStmt,";

                    if (llvm::isa<clang::ReturnStmt>(ST))
                        llvm::errs() << "ReturnStmt,";
                    if (llvm::isa<clang::GotoStmt>(ST))
                        llvm::errs() << "GotoStmt,";

                    if (llvm::isa<clang::CharacterLiteral>(ST))
                        llvm::errs() << "CharacterLiteral,";
                    if (llvm::isa<clang::IntegerLiteral>(ST))
                        llvm::errs() << "IntegerLiteral,";
                    if (llvm::isa<clang::FloatingLiteral>(ST))
                        llvm::errs() << "FloatingLiteral,";
                    if (llvm::isa<clang::FixedPointLiteral>(ST))
                        llvm::errs() << "FixedPointLiteral,";
                    if (llvm::isa<clang::ImaginaryLiteral>(ST))
                        llvm::errs() << "ImaginaryLiteral,";

                    if (llvm::isa<clang::StringLiteral>(ST))
                        llvm::errs() << "StringLiteral,";
                    if (llvm::isa<clang::CompoundLiteralExpr>(ST))
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
                }

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
                llvm::errs() << "Unaligned arguments,";

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
