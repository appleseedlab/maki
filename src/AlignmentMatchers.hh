#pragma once

#include "clang/ASTMatchers/ASTMatchFinder.h"

#include <algorithm>

namespace cpp2c
{
    using namespace clang::ast_matchers;

    // Matches all AST nodes who expand to a source range that shares
    // begin/end with the given range
    AST_POLYMORPHIC_MATCHER_P2(
        alignsWithExpansion,
        AST_POLYMORPHIC_SUPPORTED_TYPES(clang::Decl,
                                        clang::Stmt,
                                        clang::TypeLoc),
        clang::ASTContext *, Ctx,
        cpp2c::MacroExpansionNode *, Expansion)
    {

        static std::set<MacroExpansionNode *> MatchedWithFrontArg;
        static std::set<MacroExpansionNode *> MatchedWithBackArg;

        // If this expansion begins or ends with an argument expansion,
        // and we've found an almost match for this whole expansion before,
        // then stop now, since any match after that can only be a subtree
        // because arguments are expanded in their entirety
        if (MatchedWithFrontArg.find(Expansion) != MatchedWithFrontArg.end() ||
            MatchedWithBackArg.find(Expansion) != MatchedWithBackArg.end())
            return false;

        auto &SM = Ctx->getSourceManager();
        auto DefB = Expansion->DefinitionTokens.front().getLocation();
        auto DefE = Expansion->DefinitionTokens.back().getLocation();
        auto NodeSpB = SM.getSpellingLoc(Node.getBeginLoc());
        auto NodeSpE = SM.getSpellingLoc(Node.getEndLoc());
        // auto NodeExB = SM.getExpansionLoc(Node.getBeginLoc());
        auto NodeExE = SM.getExpansionLoc(Node.getEndLoc());

        bool frontAligned = false;
        if (Expansion->ArgDefBeginsWith)
        {
            frontAligned = ((!(Expansion->ArgDefBeginsWith->Tokens.empty())) &&
                            (NodeSpB ==
                             Expansion->ArgDefBeginsWith->Tokens.front()
                                 .getLocation()));
            if (frontAligned)
                MatchedWithFrontArg.insert(Expansion);
        }
        else
            frontAligned = (NodeSpB == DefB);

        bool backAligned = false;
        if (Expansion->ArgDefEndsWith)
        {
            backAligned = ((!(Expansion->ArgDefEndsWith->Tokens.empty())) &&
                           (NodeSpE ==
                            Expansion->ArgDefEndsWith->Tokens.back()
                                .getLocation()));
            if (backAligned)
                MatchedWithBackArg.insert(Expansion);
        }
        else
            backAligned = (NodeSpE == DefE);

        if (!Expansion->SpellingRange.fullyContains(NodeExE))
            return false;

        // Either the node aligns with the macro itself,
        // or one of its arguments
        return frontAligned && backAligned;
    }

    // Matches all AST nodes who span the same range that the
    // given token list spans, and for whose range every token
    // in the list is spelled
    AST_POLYMORPHIC_MATCHER_P2(
        isSpelledFromTokens,
        AST_POLYMORPHIC_SUPPORTED_TYPES(clang::Decl,
                                        clang::Stmt,
                                        clang::TypeLoc),
        clang::ASTContext *, Ctx,
        std::vector<clang::Token>, Tokens)
    {
        // First ensure that the token list is not empty, because if it is,
        // then of course it is impossible for a node to be spelled from an
        // empty token list.
        if (Tokens.empty())
            return false;

        // These sets keep track of nodes we have already matched,
        // so that we do not match their subtrees as well
        static std::set<const clang::Stmt *> MatchedStmts;
        static std::set<const clang::Decl *> MatchedDecls;
        static std::set<const clang::TypeLoc *> MatchedTypeLocs;

        auto &SM = Ctx->getSourceManager();
        auto NodeB = SM.getSpellingLoc(Node.getBeginLoc());
        auto NodeE = SM.getSpellingLoc(Node.getEndLoc());
        auto TokB = Tokens.front().getLocation();
        // Note: We do NOT use getEndLoc for the last token!
        auto TokE = Tokens.back().getLocation();
        clang::SourceRange SpellingRange(NodeB, NodeE);
        clang::SourceRange TokRange(TokB, TokE);

        // Ensure that every token in the list is included
        // in the range spanned by this AST node
        for (auto Tok : Tokens)
            if (!SpellingRange.fullyContains(Tok.getLocation()))
                return false;

        // Ensure that this node spans the same range
        // as the given token list
        if (NodeB != TokB || NodeE != TokE)
            return false;

        // Check that this node is not the subtree of an aligned node
        // that we already found.
        for (auto P : Ctx->getParents(Node))
        {
            if (auto PST = P.template get<clang::Stmt>())
            {
                if (MatchedStmts.find(PST) != MatchedStmts.end())
                    return false;
            }
            else if (auto DP = P.template get<clang::Decl>())
            {
                if (MatchedDecls.find(DP) != MatchedDecls.end())
                    return false;
            }
            else if (auto DTL = P.template get<clang::TypeLoc>())
            {
                if (MatchedTypeLocs.find(DTL) != MatchedTypeLocs.end())
                    return false;
            }
        }

        // Store this node in the set of aligned subtrees we've found
        // Since we can't know the type of the node beforehand,
        // I'm using a bit of a hack to determine the type of Node
        // on-the-fly
        DeclStmtTypeLoc DTSL(&Node);
        if (DTSL.ST)
            MatchedStmts.insert(DTSL.ST);
        else if (DTSL.D)
            MatchedDecls.insert(DTSL.D);
        else if (DTSL.TL)
            MatchedTypeLocs.insert(DTSL.TL);

        return true;
    }
}