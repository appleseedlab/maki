#include "MacroExpansionNode.hh"

#include "clang/Basic/SourceManager.h"

#include "assert.h"

namespace cpp2c
{
    void cpp2c::MacroExpansionNode::dump(llvm::raw_fd_ostream &OS,
                                         unsigned int indent)
    {
        for (unsigned int i = 0; i < indent; i++)
        {
            OS << "\t";
        }

        OS << Name << " @ depth " << Depth << "\n";

        for (auto Child : Children)
        {
            Child->dump(OS, indent + 1);
        }
    }
} // namespace cpp2c
