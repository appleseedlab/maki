#include "StmtCollectorMatchHandler.hh"

namespace cpp2c
{
    void StmtCollectorMatchHandler::run(
        const clang::ast_matchers::MatchFinder::MatchResult &Result)
    {
        if (auto ST = Result.Nodes.getNodeAs<clang::Stmt>("root"))
            Stmts.insert(ST);
        else
            assert(!"Matched a non-Stmt node");
    }
} // namespace cpp2c
