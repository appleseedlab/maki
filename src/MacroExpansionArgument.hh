#pragma once

#include "DeclStmtTypeLoc.hh"

#include "llvm/ADT/StringRef.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Lex/Token.h"

#include <vector>

namespace cpp2c
{
    class MacroExpansionArgument
    {
    public:
        // The name of the parameter this argument expands
        llvm::StringRef Name;
        // The raw tokens comprising this argument
        std::vector<clang::Token> Tokens;
        // The AST roots this argument aligns with, if any
        std::vector<cpp2c::DeclStmtTypeLoc> AlignedRoots;
        // The number of times this argument is expanded in the body
        // of its corresponding macro definition.
        // If this argument is expanded properly, then this number
        // should be equal to the number of aligned AST roots for this
        // argument
        unsigned int NumExpansions = 0;

        // Prints information the AST nodes aligned with this argument
        void dumpASTInfo(llvm::raw_fd_ostream &OS,
                         clang::SourceManager &SM,
                         const clang::LangOptions &LO);
    };
} // namespace cpp2c