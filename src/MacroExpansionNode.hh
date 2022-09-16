#pragma once

#include "DeclStmtTypeLoc.hh"
#include "clang/Basic/SourceLocation.h"

#include "llvm/ADT/StringRef.h"
#include "llvm/Support/raw_ostream.h"

#include <vector>

namespace cpp2c
{
    class MacroExpansionNode
    {
    public:
        // Internal struct that represents the tokens behind a single
        // macro expansion's argument
        struct Argument
        {
            llvm::StringRef Name;
            std::vector<clang::SourceRange> TokenRanges;
        };

        // Invocations that were directly expanded under this expansion
        std::vector<cpp2c::MacroExpansionNode *> Children;
        // The name of the expanded macro
        llvm::StringRef Name;
        // The expansion that this expansion was expanded under (if any)
        cpp2c::MacroExpansionNode *Parent;
        // The source range that the definition of this expanded macro spans
        clang::SourceRange DefinitionRange;
        // The source range that the invocation (spelling) of this expansion
        // spans.
        // This is the range of text that the developer would see when writing
        // this macro.
        // The spelling range of nested expansions is inside the definition
        // of the macro whose expansion they are nested under.
        clang::SourceRange SpellingRange;
        // How deeply nested this macro is in its expansion tree
        unsigned int Depth;
        // The AST roots of this expansion, if any
        std::vector<cpp2c::DeclStmtTypeLoc> ASTRoots;
        // The AST root this expansion is aligned with, if any
        cpp2c::DeclStmtTypeLoc *AlignedRoot = nullptr;
        // The arguments to this macro invocation, if any
        std::vector<Argument> Arguments;

        // Prints a macro expansion tree
        void dump(llvm::raw_fd_ostream &OS, unsigned int indent = 0);
        // Sets this expansion's AlignedRoot to the first of its AST roots
        // that it aligned with.
        // Returns true if a node was matched, false otherwise
        bool findAlignedRoot(clang::SourceManager &SM);
    };

} // namespace cpp2c
