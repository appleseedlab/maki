#pragma once

#include "clang/ASTMatchers/ASTMatchFinder.h"

#include "clang/Lex/Lexer.h"

#include <algorithm>

namespace cpp2c
{
    using namespace clang::ast_matchers;

    void storeChildren(cpp2c::DeclStmtTypeLoc DSTL,
                       std::set<const clang::Stmt *> &MatchedStmts,
                       std::set<const clang::Decl *> &MatchedDecls,
                       std::set<const clang::TypeLoc *> &MatchedTypeLocs)
    {
        if (DSTL.ST)
        {
            std::stack<const clang::Stmt *> Descendants;
            Descendants.push(DSTL.ST);
            while (!Descendants.empty())
            {
                auto Cur = Descendants.top();
                Descendants.pop();
                MatchedStmts.insert(Cur);
                for (auto Child : Cur->children())
                    Descendants.push(Child);
            }
        }
        else if (DSTL.D)
            MatchedDecls.insert(DSTL.D);
        else if (DSTL.TL)
            MatchedTypeLocs.insert(DSTL.TL);
    }

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
        // These sets keep track of nodes we have already matched,
        // so that we do not match their subtrees as well
        static std::set<const clang::Stmt *> MatchedStmts;
        static std::set<const clang::Decl *> MatchedDecls;
        static std::set<const clang::TypeLoc *> MatchedTypeLocs;

        auto &SM = Ctx->getSourceManager();
        auto DefB = Expansion->DefinitionTokens.front().getLocation();
        auto DefE = Expansion->DefinitionTokens.back().getLocation();
        auto NodeSpB = SM.getSpellingLoc(Node.getBeginLoc());
        auto NodeSpE = SM.getSpellingLoc(Node.getEndLoc());
        // auto NodeExB = SM.getExpansionLoc(Node.getBeginLoc());
        auto NodeExE = SM.getExpansionLoc(Node.getEndLoc());
        DeclStmtTypeLoc DSTL(&Node);

        constexpr bool debug = false;

        if (!Expansion->SpellingRange.fullyContains(NodeExE))
        {
            if (DSTL.ST && debug)
            {
                llvm::errs() << "Node mismatch <exp end not in spelling range>\n";
                DSTL.ST->dumpColor();
                llvm::errs() << "Spelling range: ";
                Expansion->SpellingRange.dump(SM);
                llvm::errs() << "Expansion end: ";
                NodeExE.dump(SM);
                llvm::errs() << "Imm macro caller loc end: ";
                SM.getImmediateMacroCallerLoc(NodeExE).dump(SM);
            }
            return false;
        }

        bool frontAligned = ((Expansion->ArgDefBeginsWith) &&
                             (!(Expansion->ArgDefBeginsWith->Tokens.empty()) &&
                              (NodeSpB ==
                               SM.getSpellingLoc(Expansion->ArgDefBeginsWith->Tokens.front()
                                                     .getLocation())))) ||
                            (NodeSpB == DefB);

        bool backAligned = ((Expansion->ArgDefEndsWith) &&
                            (!(Expansion->ArgDefEndsWith->Tokens.empty()) &&
                             (NodeSpE ==
                              SM.getSpellingLoc(Expansion->ArgDefEndsWith->Tokens.back()
                                                    .getLocation())))) ||
                           (NodeSpE == DefE);

        // Either the node aligns with the macro itself,
        // or one of its arguments.
        if (!frontAligned)
        {
            if (DSTL.ST && debug)
            {
                llvm::errs() << "Node mismatch <not front aligned>\n";
                DSTL.ST->dumpColor();
                llvm::errs() << "Node begin loc: ";
                Node.getBeginLoc().dump(SM);
                llvm::errs() << "NodeSpB: ";
                NodeSpB.dump(SM);
                llvm::errs() << "DefB: ";
                DefB.dump(SM);
                if (Expansion->ArgDefBeginsWith &&
                    !(Expansion->ArgDefBeginsWith->Tokens.empty()))
                {
                    llvm::errs() << "ArgDefBeginsWith front token loc: ";
                    Expansion->ArgDefBeginsWith->Tokens.front()
                        .getLocation()
                        .dump(SM);
                }
            }
            return false;
        }
        if (!backAligned)
        {
            if (DSTL.ST && debug)
            {
                llvm::errs() << "Node mismatch <not back aligned>\n";
                DSTL.ST->dumpColor();
            }
            return false;
        }

        // Check that this node has not been matched before
        if (DSTL.ST && MatchedStmts.find(DSTL.ST) != MatchedStmts.end())
            return false;
        else if (DSTL.D && MatchedDecls.find(DSTL.D) != MatchedDecls.end())
            return false;
        else if (DSTL.TL &&
                 MatchedTypeLocs.find(DSTL.TL) != MatchedTypeLocs.end())
            return false;

        // Check that this node is not a proper subtree of an aligned node
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

        // Store this node and its children in the set of aligned subtrees
        // we've found
        storeChildren(DSTL, MatchedStmts, MatchedDecls, MatchedTypeLocs);
        return true;
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

        constexpr bool debug = false;

        auto &SM = Ctx->getSourceManager();
        auto NodeB = SM.getFileLoc(Node.getBeginLoc());
        auto NodeE = SM.getFileLoc(Node.getEndLoc());
        auto TokB = SM.getFileLoc(Tokens.front().getLocation());
        // Note: We do NOT use getEndLoc for the last token!
        auto TokE = SM.getFileLoc(Tokens.back().getLocation());
        clang::SourceRange SpellingRange(NodeB, NodeE);
        clang::SourceRange TokFileRange(TokB, TokE);
        clang::SourceRange TokExpRange(
            SM.getExpansionLoc(Tokens.front().getLocation()),
            SM.getExpansionLoc(Tokens.back().getLocation()));
        clang::SourceRange ExpImmMacroCallerRange(
            SM.getExpansionLoc(SM.getImmediateMacroCallerLoc(Node.getBeginLoc())),
            SM.getExpansionLoc(SM.getImmediateMacroCallerLoc(Node.getEndLoc())));

        // Check that this node has not been matched before
        DeclStmtTypeLoc DSTL(&Node);
        if (DSTL.ST && MatchedStmts.find(DSTL.ST) != MatchedStmts.end())
            return false;
        else if (DSTL.D && MatchedDecls.find(DSTL.D) != MatchedDecls.end())
            return false;
        else if (DSTL.TL &&
                 MatchedTypeLocs.find(DSTL.TL) != MatchedTypeLocs.end())
            return false;

        // Ensure that every token in the list is included
        // in the range spanned by this AST node
        for (auto Tok : Tokens)
            if (!(SpellingRange
                      .fullyContains(SM.getExpansionLoc(Tok.getLocation())) ||
                  ExpImmMacroCallerRange.fullyContains(
                      SM.getExpansionLoc(Tok.getLocation()))))
            {
                DeclStmtTypeLoc DSTL(&Node);
                if (DSTL.ST && debug)
                {
                    llvm::errs() << "Node mismatch <token not in range>\n";
                    DSTL.ST->dumpColor();
                    llvm::errs() << "Spelling range: ";
                    SpellingRange.dump(SM);
                    llvm::errs() << "Token loc: ";
                    Tok.getLocation().dump(SM);
                    llvm::errs() << "TokExpRange: ";
                    TokExpRange.dump(SM);
                    llvm::errs() << "ExImmMacroCallerRange: ";
                    ExpImmMacroCallerRange.dump(SM);
                }
                return false;
            }

        // Ensure that this node spans the same range
        // as the given token list
        if (NodeB != TokB || NodeE != TokE)
        {
            DeclStmtTypeLoc DSTL(&Node);
            if (DSTL.ST && debug)
            {
                llvm::errs() << "Node mismatch <range mismatch>:\n";
                DSTL.ST->dumpColor();
                llvm::errs() << "NodeB: ";
                NodeB.dump(SM);
                llvm::errs() << "NodeE: ";
                NodeE.dump(SM);
                llvm::errs() << "TokB: ";
                TokB.dump(SM);
                llvm::errs() << "TokE: ";
                TokE.dump(SM);
            }
            return false;
        }

        // TODO: Keep track of expansion ranges of macros passed as arguments
        // to other macros
        if (DSTL.ST)
        {
            std::vector<const clang::Stmt *> descendants;
            for (auto child : DSTL.ST->children())
                descendants.push_back(child);
            while (!descendants.empty())
            {
                auto cur = descendants.back();
                descendants.pop_back();

                auto DescB = SM.getFileLoc(cur->getBeginLoc());
                auto DescE = SM.getFileLoc(cur->getEndLoc());
                clang::SourceRange FileRange(DescB, DescE);

                clang::SourceRange ExpImmMacroCallerRange(
                    SM.getExpansionLoc(SM.getImmediateMacroCallerLoc(cur->getBeginLoc())),
                    SM.getExpansionLoc(SM.getImmediateMacroCallerLoc(cur->getEndLoc())));

                if (!(TokFileRange.fullyContains(FileRange) ||
                      TokExpRange == ExpImmMacroCallerRange))
                {
                    if (debug)
                    {
                        llvm::errs() << "Node mismatch <descendant>:\n";
                        DSTL.ST->dumpColor();
                        llvm::errs() << "Descendant did not match:\n";
                        cur->dumpColor();
                        llvm::errs() << "Desc begin loc: ";
                        cur->getBeginLoc().dump(SM);
                        llvm::errs() << "Desc end loc: ";
                        cur->getEndLoc().dump(SM);
                        llvm::errs() << "Desc spelling loc: ";
                        SM.getSpellingLoc(cur->getBeginLoc()).dump(SM);
                        llvm::errs() << "Desc spelling end loc: ";
                        SM.getSpellingLoc(cur->getEndLoc()).dump(SM);
                        llvm::errs() << "Desc imm macro caller loc: ";
                        SM.getImmediateMacroCallerLoc(cur->getBeginLoc()).dump(SM);
                        llvm::errs() << "Desc imm macro caller end loc: ";
                        SM.getImmediateMacroCallerLoc(cur->getEndLoc()).dump(SM);
                        llvm::errs() << "Token loc: ";
                        Tokens.front().getLocation().dump(SM);
                        llvm::errs() << "Token end loc: ";
                        Tokens.back().getLocation().dump(SM);
                    }
                    return false;
                }

                for (auto &&child : cur->children())
                    descendants.push_back(child);
            }
        }

        // Store this node and its children in the set of aligned subtrees
        // we've found
        storeChildren(DSTL, MatchedStmts, MatchedDecls, MatchedTypeLocs);

        return true;
    }
}