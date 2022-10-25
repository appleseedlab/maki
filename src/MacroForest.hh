#pragma once

#include "MacroExpansionNode.hh"

#include "clang/Lex/PPCallbacks.h"
#include "clang/AST/ASTContext.h"

#include <vector>
#include <stack>

namespace cpp2c
{
    class MacroForest : public clang::PPCallbacks
    {
    public:
        clang::Preprocessor &PP;
        clang::ASTContext &Ctx;
        std::vector<cpp2c::MacroExpansionNode *> Expansions;

        // Whether or not the current expansion is within a macro argument
        bool InMacroArg = false;

        // The stack of previous expansions.
        // The invocations in this stack should only ever be previous
        // siblings of the current invocation, or the parent invocation
        // of the current invocation.
        std::stack<cpp2c::MacroExpansionNode *> InvocationStack;

        MacroForest(clang::Preprocessor &PP, clang::ASTContext &Ctx);

        void MacroExpands(const clang::Token &MacroNameTok,
                          const clang::MacroDefinition &MD,
                          clang::SourceRange Range,
                          const clang::MacroArgs *Args) override;
    };
} // namespace cpp2c
