#pragma once

#include "MacroExpansionNode.hh"
#include <clang/AST/ASTContext.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Lex/MacroArgs.h>
#include <clang/Lex/MacroInfo.h>
#include <clang/Lex/PPCallbacks.h>
#include <clang/Lex/Preprocessor.h>
#include <clang/Lex/Token.h>
#include <vector>

namespace maki {
class MacroForest : public clang::PPCallbacks {
public:
    clang::Preprocessor &PP;
    clang::ASTContext &Ctx;
    std::vector<maki::MacroExpansionNode *> Expansions;

    // Whether or not the current expansion is within a macro argument
    bool InMacroArg = false;

    // The stack of previous expansions. The invocations in this stack should
    // only ever be previous siblings of the current invocation, or the parent
    // invocation of the current invocation.
    std::stack<maki::MacroExpansionNode *> InvocationStack;

    MacroForest(clang::Preprocessor &PP, clang::ASTContext &Ctx);

    void MacroExpands(const clang::Token &MacroNameTok,
                      const clang::MacroDefinition &MD,
                      clang::SourceRange Range,
                      const clang::MacroArgs *Args) override;
};
} // namespace maki
