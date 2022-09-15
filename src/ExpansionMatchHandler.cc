#include "ExpansionMatchHandler.hh"

#include "clang/AST/Expr.h"

#include <assert.h>

namespace cpp2c
{
    void ExpansionMatchHandler::run(
        const clang::ast_matchers::MatchFinder::MatchResult &Result)
    {
        if (const auto D = Result.Nodes.getNodeAs<clang::Decl>("root"))
            Matches.push_back(DeclStmt(D));
        else if (const auto ST = Result.Nodes.getNodeAs<clang::Stmt>("root"))
            Matches.push_back(DeclStmt(ST));
        else
            assert(!"Matched a node that was node a Decl or Stmt");
    }
} // namespace cpp2c
