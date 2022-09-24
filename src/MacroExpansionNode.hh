#pragma once

#include "MacroExpansionArgument.hh"
#include "DeclStmtTypeLoc.hh"

#include "clang/Basic/SourceLocation.h"
#include "clang/Lex/MacroInfo.h"

#include "llvm/ADT/StringRef.h"
#include "llvm/Support/raw_ostream.h"

#include <vector>

namespace cpp2c
{
    class MacroExpansionNode
    {
    public:
        // Info about the macro this is an expansion of
        clang::MacroInfo *MI;
        // The name of the expanded macro
        llvm::StringRef Name;
        // The hash of the macro this expansion is an expansion of.
        // This hash is the source location of the macro's definition.
        std::string MacroHash;
        // The source range that the definition of this expanded macro spans
        clang::SourceRange DefinitionRange;
        // The tokens in the definition of this expanded macro
        std::vector<clang::Token> DefinitionTokens;
        // The source range that the invocation (spelling) of this expansion
        // spans.
        // This is the range of text that the developer would see when writing
        // this macro.
        // The spelling range of nested expansions is inside the definition
        // of the macro whose expansion they are nested under.
        clang::SourceRange SpellingRange;
        // How deeply nested this macro is in its expansion tree
        unsigned int Depth;
        // The expansion that this expansion was expanded under (if any)
        MacroExpansionNode *Parent;
        // Invocations that were directly expanded under this expansion
        std::vector<MacroExpansionNode *> Children;
        // The AST roots of this expansion, if any
        std::vector<DeclStmtTypeLoc> ASTRoots;
        // The AST root this expansion is aligned with, if any
        DeclStmtTypeLoc *AlignedRoot = nullptr;
        // The arguments to this macro invocation, if any
        std::vector<MacroExpansionArgument> Arguments;
        // The macro argument that that the expanded macro's definition
        // begins with.
        // If the macro's definition does not begin with an argument,
        // this is the nullptr.
        MacroExpansionArgument *ArgDefBeginsWith = nullptr;
        // The macro argument that that the expanded macro's definition
        // ends with.
        // If the macro's definition does not end with an argument,
        // this is the nullptr.
        MacroExpansionArgument *ArgDefEndsWith = nullptr;

        // Destructor should only be called on top-level expansions
        ~MacroExpansionNode();

        // Prints a macro expansion tree
        void dumpMacroInfo(llvm::raw_fd_ostream &OS, unsigned int indent = 0);
        // Prints information about the AST nodes aligned with this expansion
        // and its arguments
        void dumpASTInfo(
            llvm::raw_fd_ostream &OS,
            clang::SourceManager &SM,
            const clang::LangOptions &LO);
    };

} // namespace cpp2c
