#include "ExpansionMatchHandler.hh"
#include <cassert>
#include <clang/AST/DeclBase.h>
#include <clang/AST/Stmt.h>
#include <clang/AST/TypeLoc.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>

namespace maki {
void ExpansionMatchHandler::run(
    const clang::ast_matchers::MatchFinder::MatchResult &Result) {
    if (const auto D = Result.Nodes.getNodeAs<clang::Decl>("root")) {
        Matches.push_back(DeclStmtTypeLoc(D));
    } else if (const auto ST = Result.Nodes.getNodeAs<clang::Stmt>("root")) {
        Matches.push_back(DeclStmtTypeLoc(ST));
    } else if (const auto TL = Result.Nodes.getNodeAs<clang::TypeLoc>("root")) {
        Matches.push_back(DeclStmtTypeLoc(TL));
    } else {
        assert(!"Matched a node that was node a Decl/Stmt/TypeLoc");
    }
}
} // namespace maki
