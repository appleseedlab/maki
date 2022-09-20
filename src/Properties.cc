#include "Properties.hh"
#include "ASTUtils.hh"
#include "StmtCollectorMatchHandler.hh"

#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

// NOTE:    be sure to always use
//          unless(anyOf(implicitCastExpr(), implicitValueInitExpr()))

namespace cpp2c
{

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

    bool isHygienic(
        clang::ASTContext &Ctx,
        MacroExpansionNode *Expansion)
    {
        assert(Expansion->AlignedRoot &&
               Expansion->AlignedRoot->ST &&
               "Expansion must have an aligned Stmt root");

        using namespace clang::ast_matchers;
        // 1. Collect all local vars in the expansion
        std::vector<const clang::DeclRefExpr *> LocalVars;
        for (auto ST : matchAndCollectStmtsIn<const clang::Stmt>(
                 Ctx,
                 Expansion->AlignedRoot->ST,
                 stmt(unless(anyOf(implicitCastExpr(),
                                   implicitValueInitExpr())),
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

        using namespace clang::ast_matchers;
        // 1. Collect all expressions with side-effects in the expansion
        std::vector<const clang::Expr *> SideEffectExprs;
        for (auto ST : matchAndCollectStmtsIn<const clang::Stmt>(
                 Ctx,
                 Expansion->AlignedRoot->ST,
                 stmt(unless(anyOf(implicitCastExpr(),
                                   implicitValueInitExpr())),
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
        assert(Expansion->AlignedRoot &&
               Expansion->AlignedRoot->ST &&
               "Expansion must have an aligned Stmt root");

        using namespace clang::ast_matchers;
        // 1. Collect & exprs and side-effecting exprs
        std::vector<const clang::Expr *> LValueExprs;
        for (auto ST : matchAndCollectStmtsIn<const clang::Stmt>(
                 Ctx,
                 Expansion->AlignedRoot->ST,
                 stmt(unless(anyOf(implicitCastExpr(),
                                   implicitValueInitExpr())),
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
} // namespace cpp2c
