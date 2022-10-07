#pragma once

#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/AST/Decl.h"

#include <vector>

namespace cpp2c
{
    class DeclCollectorMatchHandler
        : public clang::ast_matchers::MatchFinder::MatchCallback
    {
    public:
        std::vector<const clang::Decl *> Decls;

        virtual void run(
            const clang::ast_matchers::MatchFinder::MatchResult &Result)
            override;
    };
} // namespace cpp2c