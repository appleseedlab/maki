#pragma once

#include "clang/ASTMatchers/ASTMatchFinder.h"

namespace cpp2c
{
    using namespace clang::ast_matchers;

    // TODO: Right now, for finding nodes aligned withe expansions,
    // we first find potentially aligned nodes, and then determine which
    // of these are the actually aligned nodes.
    // For macro arguments though, we have access to the tokens behind them,
    // and are able to directly find the aligned nodes.
    // It would be nice if we could directly find all the aligned nodes
    // for expansions as well.

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
        isSpelledFromTokens,
        AST_POLYMORPHIC_SUPPORTED_TYPES(clang::Decl,
                                        clang::Stmt,
                                        clang::TypeLoc),
        clang::ASTContext *, Ctx,
        std::vector<clang::Token>, Tokens)
    {
        auto &SM = Ctx->getSourceManager();
        auto B = SM.getSpellingLoc(Node.getBeginLoc());
        auto E = SM.getSpellingLoc(Node.getEndLoc());
        clang::SourceRange SpellingRange(B, E);

        // First ensure that this node spans the same range
        // as the given token list
        if (!(B == Tokens.front().getLocation() &&
              E == Tokens.back().getLocation()))
            return false;

        // Next, ensure that every token in the list is included
        // in the range spanned by this AST node
        for (auto Tok : Tokens)
            if (!SpellingRange.fullyContains(Tok.getLocation()))
                return false;

        return true;
    }

// Utility macro for matching AST roots of an expansion
// TODO: Change this to a function
#define MATCH_EXPANSION_ROOTS_OF(MATCHER, EXPANSION)          \
    do                                                        \
    {                                                         \
        MatchFinder Finder;                                   \
        ExpansionMatchHandler Handler;                        \
        auto Matcher = MATCHER(expandsToShareSourceRangeEdge( \
                                   &Ctx,                      \
                                   EXPANSION->SpellingRange)) \
                           .bind("root");                     \
        Finder.addMatcher(Matcher, &Handler);                 \
        Finder.matchAST(Ctx);                                 \
        for (auto M : Handler.Matches)                        \
            EXPANSION->ASTRoots.push_back(M);                 \
    } while (0)

// Utility macro for matching aligned AST roots of a macro argument
// TODO: Change this to a function
#define MATCH_ARGUMENT_ROOTS_OF(MATCHER, ARGUMENT)          \
    do                                                      \
    {                                                       \
        MatchFinder Finder;                                 \
        ExpansionMatchHandler Handler;                      \
        if (!((ARGUMENT)->Tokens.empty()))                  \
        {                                                   \
            auto Matcher = MATCHER(isSpelledFromTokens(     \
                                       &Ctx,                \
                                       (ARGUMENT)->Tokens)) \
                               .bind("root");               \
            Finder.addMatcher(Matcher, &Handler);           \
            Finder.matchAST(Ctx);                           \
            for (auto M : Handler.Matches)                  \
                (ARGUMENT)->AlignedRoots.push_back(M);      \
        }                                                   \
    } while (0)
} // namespace clang::cpp2c
