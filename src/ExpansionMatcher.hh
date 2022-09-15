#pragma once

#include "clang/ASTMatchers/ASTMatchFinder.h"

namespace cpp2c
{
    using namespace clang::ast_matchers;

    // Matches the root AST nodes of a given macro expansion.
    AST_POLYMORPHIC_MATCHER_P2(
        isMacroExpansionASTRoot,
        AST_POLYMORPHIC_SUPPORTED_TYPES(clang::Decl,
                                        clang::Stmt,
                                        clang::TypeLoc),
        clang::ASTContext *, Ctx,
        MacroExpansionNode *, Expansion)
    {
        auto &SM = Ctx->getSourceManager();
        // auto EB = Expansion->SpellingRange.getBegin();
        auto EE = Expansion->SpellingRange.getEnd();
        // auto NB = SM.getExpansionLoc(Node.getBeginLoc());
        auto NE = SM.getExpansionLoc(Node.getEndLoc());

        // If any of this AST node's parents were expanded
        // from this macro, then this AST node can't be the AST root
        // for this expansion
        for (auto Parent : Ctx->getParents(Node))
            // Confusing syntax for template get
            if (auto P = Parent.template get<clang::Decl>())
            {
                if (SM.getExpansionLoc(P->getEndLoc()) == EE)
                    return false;
            }
            else if (auto P = Parent.template get<clang::Stmt>())
            {
                if (SM.getExpansionLoc(P->getEndLoc()) == EE)
                    return false;
            }
            else if (auto P = Parent.template get<clang::TypeLoc>())
                if (SM.getExpansionLoc(P->getEndLoc()) == EE)
                    return false;

        // Ensure that this AST node is in fact expanded
        // from the given expansion
        return (EE == NE);
    }
} // namespace clang::cpp2c
