#pragma once

#include "clang/Basic/SourceLocation.h"

#include "llvm/ADT/StringRef.h"
#include "llvm/Support/raw_ostream.h"

#include <vector>

namespace cpp2c
{
    class MacroExpansionNode
    {
    public:
        // Invocations that were directly expanded under this expansion
        std::vector<cpp2c::MacroExpansionNode *> Children;
        // The name of the expanded macro
        llvm::StringRef Name;
        // The expansion that this expansion was expanded under (if any)
        cpp2c::MacroExpansionNode *Parent;
        // The source range that the definition of this expanded macro spans
        clang::SourceRange DefinitionRange;
        // The source ranges spanned by the tokens passed to invocation
        std::vector<clang::SourceRange> ArgPreExpRanges;
        // The source range that the invocation (spelling) of this expansion
        // spans.
        // This is the range of text that the developer would see when writing
        // this macro.
        // The spelling range of nested expansions is inside the definition
        // of the macro whose expansion they are nested under.
        clang::SourceRange SpellingRange;
        // How deeply nested this macro is in its expansion tree
        unsigned int Depth;

        // Prints a macro expansion tree
        void dump(llvm::raw_fd_ostream &OS,
                  unsigned int indent = 0);
    };

} // namespace cpp2c
