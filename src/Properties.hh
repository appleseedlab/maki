#pragma once

#include "MacroExpansionNode.hh"
#include "StmtCollectorMatchHandler.hh"

#include "clang/AST/ASTContext.h"

namespace cpp2c
{

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
        MacroExpansionNode *Expansion);

    bool isParameterSideEffectFree(
        clang::ASTContext &Ctx,
        MacroExpansionNode *Expansion);

    bool isLValueIndependent(
        clang::ASTContext &Ctx,
        MacroExpansionNode *Expansion);
} // namespace cpp2c
