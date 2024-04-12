#pragma once

#include "clang/AST/Stmt.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

#include <set>

namespace cpp2c {
class StmtCollectorMatchHandler
    : public clang::ast_matchers::MatchFinder::MatchCallback {
public:
    std::set<const clang::Stmt *> Stmts;

    virtual void
    run(const clang::ast_matchers::MatchFinder::MatchResult &Result) override;
};
} // namespace cpp2c