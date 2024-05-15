#pragma once

#include "DeclStmtTypeLoc.hh"
#include "MacroExpansionNode.hh"

#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Lex/Lexer.h"
#include <clang/AST/DeclBase.h>
#include <clang/AST/Stmt.h>
#include <clang/AST/TypeLoc.h>

namespace maki {
using namespace clang::ast_matchers;

void storeChildren(maki::DeclStmtTypeLoc DSTL,
                   std::set<const clang::Stmt *> &MatchedStmts,
                   std::set<const clang::Decl *> &MatchedDecls,
                   std::set<const clang::TypeLoc *> &MatchedTypeLocs);

// Matches all AST nodes that align perfectly with the body of the given
// macro expansion.
// Only tested to work with top-level, non-argument expansions.
AST_POLYMORPHIC_MATCHER_P2(
    alignsWithExpansion,
    AST_POLYMORPHIC_SUPPORTED_TYPES(clang::Decl, clang::Stmt, clang::TypeLoc),
    clang::ASTContext *, Ctx, maki::MacroExpansionNode *, Expansion) {
    // Can't match an expansion with no tokens
    if (Expansion->DefinitionTokens.empty()) {
        return false;
    }

    // Can't match an expansion with an invalid location
    if (Node.getBeginLoc().isInvalid() || Node.getEndLoc().isInvalid()) {
        return false;
    }

    auto DefinitionBegin = Expansion->DefinitionTokens.front().getLocation();
    auto DefinitionEnd = Expansion->DefinitionTokens.back().getLocation();
    if (DefinitionBegin.isInvalid() || DefinitionEnd.isInvalid()) {
        return false;
    }

    // These sets keep track of nodes we have already matched,
    // so that we do not match their subtrees as well
    static std::set<const clang::Stmt *> MatchedStmts;
    static std::set<const clang::Decl *> MatchedDecls;
    static std::set<const clang::TypeLoc *> MatchedTypeLocs;

    // Collect a bunch of SourceLocation information up front that may be
    // useful later

    auto &SM = Ctx->getSourceManager();

    auto NodeSpellingBegin = SM.getSpellingLoc(Node.getBeginLoc());
    auto NodeSpellingEnd = SM.getSpellingLoc(Node.getEndLoc());
    auto NodeExpansionBegin = SM.getExpansionLoc(Node.getBeginLoc());
    auto NodeExpansionEnd = SM.getExpansionLoc(Node.getEndLoc());
    auto ImmediateMacroCallerSpellingBegin =
        SM.getSpellingLoc(SM.getImmediateMacroCallerLoc(Node.getBeginLoc()));
    auto ImmediateMacroCallerSpellingEnd =
        SM.getSpellingLoc(SM.getImmediateMacroCallerLoc(Node.getEndLoc()));
    auto ImmediateMacroCallerExpansionBegin =
        SM.getExpansionLoc(SM.getImmediateMacroCallerLoc(Node.getBeginLoc()));
    auto ImmediateMacroCallerExpansionEnd =
        SM.getExpansionLoc(SM.getImmediateMacroCallerLoc(Node.getEndLoc()));
    DeclStmtTypeLoc DSTL(&Node);

    static const constexpr bool debug = false;

    // Preliminary check to ensure that the spelling range of the top
    // level expansion includes the expansion range of the given node
    // NOTE: We may not need this check, but I should doublecheck
    if (!Expansion->SpellingRange.fullyContains(NodeExpansionEnd)) {
        if (debug) {
            llvm::errs() << "Node mismatch <exp end not in expansion "
                            "spelling range>\n";
            if (DSTL.ST) {
                DSTL.ST->dumpColor();
            } else if (DSTL.D) {
                DSTL.D->dumpColor();
            } else if (DSTL.TL) {
                auto QT = DSTL.TL->getType();
                if (!QT.isNull()) {
                    QT.dump();
                } else {
                    llvm::errs() << "<Null type>\n";
                }
            }
            llvm::errs() << "Expansion spelling range: ";
            Expansion->SpellingRange.dump(SM);
            llvm::errs() << "NodeSpellingBegin: ";
            NodeSpellingBegin.dump(SM);
            llvm::errs() << "NodeSpellingEnd: ";
            NodeSpellingEnd.dump(SM);
            llvm::errs() << "Expansion end: ";
            NodeExpansionEnd.dump(SM);
            llvm::errs() << "Imm macro caller loc: ";
            SM.getImmediateMacroCallerLoc(Node.getBeginLoc()).dump(SM);
            llvm::errs() << "Imm macro caller loc end: ";
            SM.getImmediateMacroCallerLoc(Node.getEndLoc()).dump(SM);
        }
        return false;
    }

    // Check that the beginning of the node we are considering
    // aligns with the beginning of the macro expansion.
    // There are three cases to consider:
    // 1. The node begins with a token that that comes directly from
    //    the body of the macro's definition
    // 2. The node begins with a token that comes from an expansion of
    //    an argument passed to the call to the macro
    // 3. The node begins with a token that comes from an expansion of a
    //    nested macro invocation in the body of the macro's definition

    // Set up case 2
    clang::SourceLocation ArgBegin;
    if (auto Arg = Expansion->ArgDefBeginsWith) {
        if (!Arg->Tokens.empty()) {
            auto L = Arg->Tokens.front().getLocation();
            if (L.isValid()) {
                ArgBegin = SM.getSpellingLoc(L);
            }
        }
    }
    clang::SourceLocation ArgEnd;
    if (auto Arg = Expansion->ArgDefEndsWith) {
        if (!Arg->Tokens.empty()) {
            auto L = Arg->Tokens.back().getLocation();
            if (L.isValid()) {
                ArgEnd = SM.getSpellingLoc(L);
            }
        }
    }

    // Set up case 3
    clang::SourceLocation Begin = Node.getBeginLoc();
    while (SM.getImmediateMacroCallerLoc(Begin).isMacroID() &&
           SM.getImmediateMacroCallerLoc(Begin).isValid()) {
        Begin = SM.getImmediateMacroCallerLoc(Begin);
    }
    Begin = SM.getSpellingLoc(Begin);

    clang::SourceLocation End = Node.getEndLoc();
    while (SM.getImmediateMacroCallerLoc(End).isMacroID() &&
           SM.getImmediateMacroCallerLoc(End).isValid()) {
        End = SM.getImmediateMacroCallerLoc(End);
    }
    End = SM.getSpellingLoc(End);

    bool frontAligned =
        // Case 1
        (DefinitionBegin == NodeSpellingBegin) ||
        // Case 2
        (ArgBegin.isValid() && ArgBegin == NodeSpellingBegin) ||
        // Case 3
        (DefinitionBegin == Begin);

    bool backAligned =
        // Case 1
        (NodeSpellingEnd == DefinitionEnd) ||
        // Case 2
        (ArgEnd.isValid() && ArgEnd == NodeSpellingEnd) ||
        // Case 3
        (DefinitionEnd == End);

    // Either the node aligns with the macro itself,
    // or one of its arguments.
    if (!frontAligned) {
        if (debug) {
            llvm::errs() << "Node mismatch <not front aligned>\n";
            if (DSTL.ST)
                DSTL.ST->dumpColor();
            else if (DSTL.D)
                DSTL.D->dumpColor();
            else if (DSTL.TL) {
                auto QT = DSTL.TL->getType();
                if (!QT.isNull())
                    QT.dump();
                else
                    llvm::errs() << "<Null type>\n";
            }

            auto ImmSpellingLoc =
                SM.getImmediateSpellingLoc(Node.getBeginLoc());

            llvm::errs() << "Node begin loc: ";
            Node.getBeginLoc().dump(SM);
            llvm::errs() << "NodeExpansionBegin: ";
            NodeExpansionBegin.dump(SM);
            llvm::errs() << "NodeSpellingBegin: ";
            NodeSpellingBegin.dump(SM);
            auto L1 = SM.getImmediateMacroCallerLoc(Node.getBeginLoc());
            auto L2 = SM.getImmediateMacroCallerLoc(L1);
            auto L3 = SM.getImmediateMacroCallerLoc(L2);
            auto L4 = SM.getImmediateMacroCallerLoc(L3);
            llvm::errs()
                << "Imm macro caller loc x1 (macro id: " << L1.isMacroID()
                << "): ";
            L1.dump(SM);
            llvm::errs()
                << "Imm macro caller loc x2 (macro id: " << L2.isMacroID()
                << "): ";
            L2.dump(SM);
            llvm::errs()
                << "Imm macro caller loc x3 (macro id: " << L3.isMacroID()
                << "): ";
            L3.dump(SM);
            llvm::errs()
                << "Imm macro caller loc x4 (macro id: " << L4.isMacroID()
                << "): ";
            L4.dump(SM);
            llvm::errs() << "Top macro caller loc: ";
            SM.getTopMacroCallerLoc(Node.getBeginLoc()).dump(SM);
            llvm::errs() << "Imm macro caller expansion loc: ";
            ImmediateMacroCallerExpansionBegin.dump(SM);
            llvm::errs() << "Imm macro caller spelling loc: ";
            ImmediateMacroCallerSpellingBegin.dump(SM);
            llvm::errs() << "Immediate spelling loc: ";
            ImmSpellingLoc.dump(SM);
            llvm::errs() << "DefinitionBegin: ";
            DefinitionBegin.dump(SM);
            if (Expansion->ArgDefBeginsWith &&
                !(Expansion->ArgDefBeginsWith->Tokens.empty())) {
                llvm::errs() << "ArgDefBeginsWith front token loc: ";
                Expansion->ArgDefBeginsWith->Tokens.front().getLocation().dump(
                    SM);
            }
        }
        return false;
    }
    if (!backAligned) {
        if (debug) {
            llvm::errs() << "Node mismatch <not back aligned>\n";
            if (DSTL.ST)
                DSTL.ST->dumpColor();
            else if (DSTL.D)
                DSTL.D->dumpColor();
            else if (DSTL.TL) {
                auto QT = DSTL.TL->getType();
                if (!QT.isNull())
                    QT.dump();
                else
                    llvm::errs() << "<Null type>\n";
            }

            auto ImmSpellingEndLoc =
                SM.getImmediateSpellingLoc(Node.getEndLoc());

            llvm::errs() << "Node end loc: ";
            Node.getEndLoc().dump(SM);
            llvm::errs() << "NodeExpansionEnd: ";
            NodeExpansionEnd.dump(SM);
            llvm::errs() << "NodeSpellingEnd: ";
            NodeSpellingEnd.dump(SM);
            llvm::errs() << "Imm macro caller end loc: ";
            SM.getImmediateMacroCallerLoc(Node.getEndLoc()).dump(SM);
            llvm::errs() << "Imm macro caller expansion end loc: ";
            ImmediateMacroCallerExpansionEnd.dump(SM);
            llvm::errs() << "Imm macro caller spelling end loc: ";
            ImmediateMacroCallerSpellingEnd.dump(SM);
            llvm::errs() << "Immediate spelling end loc: ";
            ImmSpellingEndLoc.dump(SM);
            llvm::errs() << "DefinitionEnd: ";
            DefinitionEnd.dump(SM);
            if (Expansion->ArgDefEndsWith &&
                !(Expansion->ArgDefEndsWith->Tokens.empty())) {
                llvm::errs() << "ArgDefEndsWith back token loc: ";
                Expansion->ArgDefEndsWith->Tokens.back().getLocation().dump(SM);
            }
        }
        return false;
    }

    // Check that this node has not been matched before
    bool foundNodeBefore = false;
    if (DSTL.ST && MatchedStmts.find(DSTL.ST) != MatchedStmts.end()) {
        foundNodeBefore = true;
    } else if (DSTL.D && MatchedDecls.find(DSTL.D) != MatchedDecls.end()) {
        foundNodeBefore = true;
    } else if (DSTL.TL &&
               MatchedTypeLocs.find(DSTL.TL) != MatchedTypeLocs.end()) {
        foundNodeBefore = true;
    }
    if (foundNodeBefore) {
        if (debug) {
            llvm::errs() << "Found node before\n";
            if (DSTL.ST) {
                DSTL.ST->dumpColor();
            } else if (DSTL.D) {
                DSTL.D->dumpColor();
            } else if (DSTL.TL) {
                auto QT = DSTL.TL->getType();
                if (!QT.isNull()) {
                    QT.dump();
                } else {
                    llvm::errs() << "<Null type>\n";
                }
            }
        }
        return false;
    }

    // Check that this node is not a proper subtree of an aligned node
    // that we already found.
    bool foundParentBefore = false;
    for (auto P : Ctx->getParents(Node)) {
        if (auto PST = P.template get<clang::Stmt>()) {
            if (MatchedStmts.find(PST) != MatchedStmts.end()) {
                foundParentBefore = true;
            }
        } else if (auto DP = P.template get<clang::Decl>()) {
            if (MatchedDecls.find(DP) != MatchedDecls.end()) {
                foundParentBefore = true;
            }
        } else if (auto DTL = P.template get<clang::TypeLoc>()) {
            if (MatchedTypeLocs.find(DTL) != MatchedTypeLocs.end()) {
                foundParentBefore = true;
            }
        }
    }
    if (foundParentBefore) {
        if (debug) {
            llvm::errs() << "Found parent before\n";
        }
        return false;
    }

    // Store this node and its children in the set of aligned subtrees
    // we've found
    storeChildren(DSTL, MatchedStmts, MatchedDecls, MatchedTypeLocs);

    if (debug) {
        llvm::errs() << "Matched " << Expansion->Name << " with:\n";
        if (DSTL.ST) {
            DSTL.ST->dumpColor();
        } else if (DSTL.D) {
            DSTL.D->dumpColor();
        } else if (DSTL.TL) {
            auto QT = DSTL.TL->getType();
            if (!QT.isNull()) {
                QT.dump();
            } else {
                llvm::errs() << "<Null type>\n";
            }
        }
    }
    return true;
}

// Matches all AST nodes who span the same range that the
// given token list spans, and for whose range every token
// in the list is spelled
AST_POLYMORPHIC_MATCHER_P2(
    isSpelledFromTokens,
    AST_POLYMORPHIC_SUPPORTED_TYPES(clang::Decl, clang::Stmt, clang::TypeLoc),
    clang::ASTContext *, Ctx, std::vector<clang::Token>, Tokens) {
    // First ensure that the token list is not empty, because if it is,
    // then of course it is impossible for a node to be spelled from an
    // empty token list.
    if (Tokens.empty()) {
        return false;
    }

    auto &SM = Ctx->getSourceManager();

    auto NodeBegin = SM.getFileLoc(Node.getBeginLoc());
    auto NodeEnd = SM.getFileLoc(Node.getEndLoc());
    if (NodeBegin.isInvalid() || NodeEnd.isInvalid()) {
        return false;
    }

    auto TokenBegin = SM.getFileLoc(Tokens.front().getLocation());
    // Note: We do NOT use getEndLoc for the last token!
    auto TokE = SM.getFileLoc(Tokens.back().getLocation());
    if (TokenBegin.isInvalid() || TokE.isInvalid()) {
        return false;
    }

    auto NodeImmediateMacroCallerBegin =
        SM.getImmediateMacroCallerLoc(Node.getBeginLoc());
    auto NodeImmediateMacroCallerEnd =
        SM.getImmediateMacroCallerLoc(Node.getEndLoc());

    if (NodeImmediateMacroCallerBegin.isInvalid() ||
        NodeImmediateMacroCallerEnd.isInvalid()) {
        return false;
    }

    // These sets keep track of nodes we have already matched,
    // so that we do not match their subtrees as well
    static std::set<const clang::Stmt *> MatchedStmts;
    static std::set<const clang::Decl *> MatchedDecls;
    static std::set<const clang::TypeLoc *> MatchedTypeLocs;

    static const constexpr bool debug = false;

    clang::SourceRange SpellingRange(NodeBegin, NodeEnd);
    clang::SourceRange TokenFileRange(TokenBegin, TokE);
    clang::SourceRange TokenExpansionRange(SM.getExpansionLoc(TokenBegin),
                                           SM.getExpansionLoc(TokE));
    clang::SourceRange ExpansionImmediateMacroCallerRange(
        SM.getExpansionLoc(NodeImmediateMacroCallerBegin),
        SM.getExpansionLoc(NodeImmediateMacroCallerEnd));

    // Check that this node has not been matched before
    DeclStmtTypeLoc DSTL(&Node);
    if (DSTL.ST && MatchedStmts.find(DSTL.ST) != MatchedStmts.end()) {
        return false;
    } else if (DSTL.D && MatchedDecls.find(DSTL.D) != MatchedDecls.end()) {
        return false;
    } else if (DSTL.TL &&
               MatchedTypeLocs.find(DSTL.TL) != MatchedTypeLocs.end()) {
        return false;
    }

    // Ensure that every token in the list is included
    // in the range spanned by this AST node
    for (auto Tok : Tokens) {
        if (!(SpellingRange.fullyContains(
                  SM.getExpansionLoc(Tok.getLocation())) ||
              ExpansionImmediateMacroCallerRange.fullyContains(
                  SM.getExpansionLoc(Tok.getLocation())))) {
            if (DSTL.ST && debug) {
                llvm::errs() << "Node mismatch <token not in range>\n";
                DSTL.ST->dumpColor();
                llvm::errs() << "Spelling range: ";
                SpellingRange.dump(SM);
                llvm::errs() << "Token loc: ";
                Tok.getLocation().dump(SM);
                llvm::errs() << "TokenExpansionRange: ";
                TokenExpansionRange.dump(SM);
                llvm::errs() << "ExImmMacroCallerRange: ";
                ExpansionImmediateMacroCallerRange.dump(SM);
            }
            return false;
        }
    }

    // Ensure that this node spans the same range
    // as the given token list
    if (NodeBegin != TokenBegin || NodeEnd != TokE) {
        if (DSTL.ST && debug) {
            llvm::errs() << "Node mismatch <range mismatch>:\n";
            DSTL.ST->dumpColor();
            llvm::errs() << "NodeBegin: ";
            NodeBegin.dump(SM);
            llvm::errs() << "NodeEnd: ";
            NodeEnd.dump(SM);
            llvm::errs() << "TokenBegin: ";
            TokenBegin.dump(SM);
            llvm::errs() << "TokE: ";
            TokE.dump(SM);
        }
        return false;
    }

    // TODO: Keep track of expansion ranges of macros passed as arguments
    // to other macros
    if (DSTL.ST) {
        std::vector<const clang::Stmt *> descendants;
        for (auto &&child : DSTL.ST->children()) {
            // if (child) /* child should not be null */
            descendants.push_back(child);
        }
        while (!descendants.empty()) {
            auto cur = descendants.back();
            descendants.pop_back();

            // TODO: Might want to return false instead of continuing here

            if (!cur) {
                continue;
            }

            auto CurBegin = cur->getEndLoc();
            auto CurE = cur->getEndLoc();

            if (CurBegin.isInvalid() || CurE.isInvalid()) {
                continue;
            }

            auto CurFileLocBegin = SM.getFileLoc(CurBegin);
            auto CurFileLocEnd = SM.getFileLoc(CurE);

            if (CurFileLocBegin.isInvalid() || CurFileLocEnd.isInvalid()) {
                continue;
            }

            clang::SourceRange FileRange(CurFileLocBegin, CurFileLocEnd);

            auto CurrentImmedeiateMacroCallerBegin =
                SM.getImmediateMacroCallerLoc(CurBegin);
            auto CurrentImmediateMacroCallerEnd =
                SM.getImmediateMacroCallerLoc(CurE);

            if (CurrentImmedeiateMacroCallerBegin.isInvalid() ||
                CurrentImmediateMacroCallerEnd.isInvalid()) {
                continue;
            }

            auto CurrentImmedeiateMacroCallerExpansionBegin =
                SM.getExpansionLoc(CurrentImmedeiateMacroCallerBegin);
            auto CurrentImmedeiateMacroCallerExpansionEnd =
                SM.getExpansionLoc(CurrentImmediateMacroCallerEnd);

            if (CurrentImmedeiateMacroCallerExpansionBegin.isInvalid() ||
                CurrentImmedeiateMacroCallerExpansionEnd.isInvalid()) {
                continue;
            }

            clang::SourceRange CurrentImmediateMacroCallerExpansionRange(
                CurrentImmedeiateMacroCallerExpansionBegin,
                CurrentImmedeiateMacroCallerExpansionEnd);

            if (!(TokenFileRange.fullyContains(FileRange) ||
                  TokenExpansionRange ==
                      CurrentImmediateMacroCallerExpansionRange)) {
                if (debug) {
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
                // if (child) /* child should not be null */
                descendants.push_back(child);
        }
    }

    // Store this node and its children in the set of aligned subtrees
    // we've found
    storeChildren(DSTL, MatchedStmts, MatchedDecls, MatchedTypeLocs);

    return true;
}

void findAlignedASTNodesForExpansion(maki::MacroExpansionNode *Exp,
                                     clang::ASTContext &Ctx);
}
