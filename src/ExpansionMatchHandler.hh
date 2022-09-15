#pragma once

#include "MacroExpansionNode.hh"

#include "clang/ASTMatchers/ASTMatchFinder.h"

#include <vector>

namespace cpp2c
{
    class ExpansionMatchHandler
        : public clang::ast_matchers::MatchFinder::MatchCallback
    {
    private:
        MacroExpansionNode *Expansion;

    public:
        ExpansionMatchHandler(MacroExpansionNode *Expansion);

        virtual void run(
            const clang::ast_matchers::MatchFinder::MatchResult &Result)
            override;
    };
} // namespace cpp2c