#include "StmtCollectorMatchHandler.hh"
#include <clang/AST/Stmt.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>

namespace maki {
void StmtCollectorMatchHandler::run(
    const clang::ast_matchers::MatchFinder::MatchResult &Result) {
    if (auto ST = Result.Nodes.getNodeAs<clang::Stmt>("root")) {
        Stmts.insert(ST);
    } else {
        assert(!"Matched a non-Stmt node");
    }
}
} // namespace maki
