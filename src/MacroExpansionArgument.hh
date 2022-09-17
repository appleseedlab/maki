#pragma once

#include "DeclStmtTypeLoc.hh"

#include "clang/Lex/Token.h"
#include "clang/Basic/SourceLocation.h"
#include "llvm/ADT/StringRef.h"

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
        // The source ranges spanned by the tokens composing this argument
        std::vector<clang::SourceRange> TokenRanges;
        // The AST roots of this argument, if any
        std::vector<cpp2c::DeclStmtTypeLoc> ASTRoots;
        // The AST roots this argument is aligned with, if any
        std::vector<cpp2c::DeclStmtTypeLoc> AlignedRoots;

        // Prints information about this argument
        void dump(llvm::raw_fd_ostream &OS);
    };
} // namespace cpp2c