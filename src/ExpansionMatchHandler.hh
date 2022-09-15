#pragma once

#include "DeclStmt.hh"

#include "clang/ASTMatchers/ASTMatchFinder.h"

#include <vector>

namespace cpp2c
{
    class ExpansionMatchHandler
        : public clang::ast_matchers::MatchFinder::MatchCallback
    {
    public:
        std::vector<DeclStmt> Matches;

        virtual void run(
            const clang::ast_matchers::MatchFinder::MatchResult &Result)
            override;
    };
} // namespace cpp2c