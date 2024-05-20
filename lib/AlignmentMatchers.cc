#include "AlignmentMatchers.hh"
#include "DeclStmtTypeLoc.hh"
#include "ExpansionMatchHandler.hh"
#include <clang/AST/ASTContext.h>
#include <clang/ASTMatchers/ASTMatchersInternal.h>
#include <vector>

namespace maki {

void storeChildren(maki::DeclStmtTypeLoc DSTL,
                   std::set<const clang::Stmt *> &MatchedStmts,
                   std::set<const clang::Decl *> &MatchedDecls,
                   std::set<const clang::TypeLoc *> &MatchedTypeLocs) {
    if (DSTL.ST) {
        // Traverse the Stmt tree using DFS
        std::stack<const clang::Stmt *> Descendants;
        Descendants.push(DSTL.ST);
        while (!Descendants.empty()) {
            auto Cur = Descendants.top();
            Descendants.pop();
            if (nullptr != Cur) {
                MatchedStmts.insert(Cur);
                for (auto &&child : Cur->children()) {
                    // if (nullptr != child) /* child should not be null */
                    Descendants.push(child);
                }
            }
        }
    } else if (DSTL.D) {
        MatchedDecls.insert(DSTL.D);
    }
    // else if (DSTL.TL)
    // {
    //     // TODO: Determine why this must be commented out to be able to
    //     //       correctly match TypeLocs
    //     // llvm::errs() << "Inserting:\n";
    // {
    //     auto QT = DSTL.TL->getType();
    //     if (!QT.isNull())
    //         QT.dump();
    //     else
    //         llvm::errs() << "<Null type>\n";
    // }
    //     MatchedTypeLocs.insert(DSTL.TL);
    // }
}

template <typename T>
std::vector<DeclStmtTypeLoc>
matchNodes(clang::ast_matchers::internal::Matcher<T> Matcher,
           clang::ASTContext &Ctx) {
    MatchFinder Finder;
    ExpansionMatchHandler MatchHandler;
    Finder.addMatcher(Matcher, &MatchHandler);
    Finder.matchAST(Ctx);
    return MatchHandler.Matches;
}

void findAlignedASTNodesForExpansion(maki::MacroExpansionNode *Expansion,
                                     clang::ASTContext &Ctx) {
    using namespace clang::ast_matchers;
    // Find AST nodes aligned with the entire expansion.

    // Match stmts
    auto StmtMatches = matchNodes(
        stmt(unless(anyOf(implicitCastExpr(), implicitValueInitExpr())),
             alignsWithExpansion(&Ctx, Expansion))
            .bind("root"),
        Ctx);
    Expansion->ASTRoots.insert(Expansion->ASTRoots.end(), StmtMatches.begin(),
                               StmtMatches.end());

    // Match decls
    auto DeclMatches = matchNodes(
        decl(alignsWithExpansion(&Ctx, Expansion)).bind("root"), Ctx);
    Expansion->ASTRoots.insert(Expansion->ASTRoots.end(), DeclMatches.begin(),
                               DeclMatches.end());

    // Match type locs
    auto TypeLocMatches = matchNodes(
        typeLoc(alignsWithExpansion(&Ctx, (Expansion))).bind("root"), Ctx);
    Expansion->ASTRoots.insert(Expansion->ASTRoots.end(),
                               TypeLocMatches.begin(), TypeLocMatches.end());

    // If the expansion only aligns with one node, then set this
    // as its aligned root
    if (1 == Expansion->ASTRoots.size()) {
        Expansion->AlignedRoot = &Expansion->ASTRoots.front();
    } else {
        Expansion->AlignedRoot = nullptr;
    }

    //// Find AST nodes aligned with each of the expansion's arguments
    for (auto &&Arg : Expansion->Arguments) {
        // Match stmts
        auto StmtMatches = matchNodes(
            stmt(unless(anyOf(implicitCastExpr(), implicitValueInitExpr())),
                 isSpelledFromTokens(&Ctx, Arg.Tokens))
                .bind("root"),
            Ctx);
        Arg.AlignedRoots.insert(Arg.AlignedRoots.end(), StmtMatches.begin(),
                                StmtMatches.end());
        // Match decls
        auto DeclMatches = matchNodes(
            decl(isSpelledFromTokens(&Ctx, Arg.Tokens)).bind("root"), Ctx);
        Arg.AlignedRoots.insert(Arg.AlignedRoots.end(), DeclMatches.begin(),
                                DeclMatches.end());

        auto TypeLocMatches = matchNodes(
            typeLoc(isSpelledFromTokens(&Ctx, Arg.Tokens)).bind("root"), Ctx);
        Arg.AlignedRoots.insert(Arg.AlignedRoots.end(), TypeLocMatches.begin(),
                                TypeLocMatches.end());
    }
}
} // namespace maki
