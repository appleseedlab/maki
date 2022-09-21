#include "MacroExpansionNode.hh"

#include "clang/Basic/SourceManager.h"

#include "assert.h"

namespace cpp2c
{

    void MacroExpansionNode::dumpMacroInfo(llvm::raw_fd_ostream &OS,
                                           unsigned int indent)
    {
        for (unsigned int i = 0; i < indent; i++)
        {
            OS << "\t";
        }

        OS << Name << " @ depth " << Depth << "\n";

        for (auto Child : Children)
        {
            Child->dumpMacroInfo(OS, indent + 1);
        }
    }

    void MacroExpansionNode::dumpASTInfo(
        llvm::raw_fd_ostream &OS,
        clang::SourceManager &SM,
        const clang::LangOptions &LO)
    {
        OS << "Top level expansion of " << Name << "\n";

        OS << "Aligned root: \n";
        if (AlignedRoot)
            AlignedRoot->dump();
        else
            OS << "None\n";

        if (!Arguments.empty())
            for (auto &&Arg : Arguments)
                Arg.dumpASTInfo(OS, SM, LO);
        else
            OS << "No arguments\n";
    }
} // namespace cpp2c
