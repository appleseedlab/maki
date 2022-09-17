#pragma once

#include "clang/ASTMatchers/ASTMatchFinder.h"

namespace cpp2c
{
    using namespace clang::ast_matchers;

    // Matches all AST nodes who expand to a source range that shares
    // begin/end with the given range
    AST_POLYMORPHIC_MATCHER_P2(
        expandsToShareSourceRangeEdge,
        AST_POLYMORPHIC_SUPPORTED_TYPES(clang::Decl,
                                        clang::Stmt,
                                        clang::TypeLoc),
        clang::ASTContext *, Ctx,
        clang::SourceRange, Range)
    {
        auto &SM = Ctx->getSourceManager();
        auto EB = Range.getBegin();
        auto EE = Range.getEnd();
        auto NB = SM.getExpansionLoc(Node.getBeginLoc());
        auto NE = SM.getExpansionLoc(Node.getEndLoc());

        // Check whether this AST node's beginning or ending loc
        // matches that of the spelling range's
        return (EB == NB) || (EE == NE);
    }

    AST_POLYMORPHIC_MATCHER_P2(
        spellsToShareSourceRangeEdge,
        AST_POLYMORPHIC_SUPPORTED_TYPES(clang::Decl,
                                        clang::Stmt,
                                        clang::TypeLoc),
        clang::ASTContext *, Ctx,
        clang::SourceRange, Range)
    {
        auto &SM = Ctx->getSourceManager();
        auto EB = Range.getBegin();
        auto EE = Range.getEnd();
        auto NB = SM.getSpellingLoc(Node.getBeginLoc());
        auto NE = SM.getSpellingLoc(Node.getEndLoc());

        // Check whether this AST node's beginning or ending loc
        // matches that of the spelling range's
        return (EB == NB) || (EE == NE);
    }

// TODO: Instead of having two difference callback classes for
// expansions and arguments, we should just have one that collects
// its results in a vec, and then has a method for accessing those results

// Utility macro for matching AST roots of an expansion
// TODO: Change this to a method for MacroExpansionNode
#define MATCH_EXPANSION_ROOTS_OF(MATCHER, EXPANSION)          \
    do                                                        \
    {                                                         \
        MatchFinder Finder;                                   \
        ExpansionMatchHandler Handler(EXPANSION);             \
        auto Matcher = MATCHER(expandsToShareSourceRangeEdge( \
                                   &Ctx,                      \
                                   EXPANSION->SpellingRange)) \
                           .bind("root");                     \
        Finder.addMatcher(Matcher, &Handler);                 \
        Finder.matchAST(Ctx);                                 \
    } while (0)

// TODO: The following macro doesn't quite work
// I think for matching arguments, we need to check expansion range edges,
// not spelling range edges.
// Yes, actually now that I think about it that makes sense.
// I should also spend some time thinking of a more general abstraction as well
#define MATCH_ARGUMENT_ROOTS_OF(MATCHER, ARGUMENT)               \
    do                                                           \
    {                                                            \
        MatchFinder Finder;                                      \
        ExpansionArgumentMatchHandler Handler(ARGUMENT);         \
        auto B = (ARGUMENT)->TokenRanges.front().getBegin();     \
        auto E = (ARGUMENT)->TokenRanges.back().getEnd();        \
        clang::SourceRange Range(B, E);                          \
        if (!((ARGUMENT)->TokenRanges.empty()))                  \
        {                                                        \
            auto Matcher = MATCHER(spellsToShareSourceRangeEdge( \
                                       &Ctx,                     \
                                       Range))                   \
                               .bind("root");                    \
            Finder.addMatcher(Matcher, &Handler);                \
            Finder.matchAST(Ctx);                                \
        }                                                        \
    } while (0)
} // namespace clang::cpp2c
