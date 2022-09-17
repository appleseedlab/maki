#pragma once

#include "MacroExpansionArgument.hh"

#include "clang/ASTMatchers/ASTMatchFinder.h"

#include <vector>

namespace cpp2c
{
    class ExpansionArgumentMatchHandler
        : public clang::ast_matchers::MatchFinder::MatchCallback
    {
    private:
        MacroExpansionArgument *Argument;

    public:
        ExpansionArgumentMatchHandler(MacroExpansionArgument *Argument);

        virtual void run(
            const clang::ast_matchers::MatchFinder::MatchResult &Result)
            override;
    };
} // namespace cpp2c