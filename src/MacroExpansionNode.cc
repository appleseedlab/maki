#include "MacroExpansionNode.hh"

#include "clang/Basic/SourceManager.h"

#include "assert.h"

#include <queue>

namespace cpp2c
{

    MacroExpansionNode::~MacroExpansionNode()
    {
        for (auto &&Child : Children)
            delete Child;
    }

    static inline void printIndent(llvm::raw_fd_ostream &OS,
                                   unsigned int indent)
    {
        for (unsigned int i = 0; i < indent; i++)
            OS << "\t";
    }

    void MacroExpansionNode::dumpMacroInfo(llvm::raw_fd_ostream &OS,
                                           unsigned int indent)
    {
        printIndent(OS, indent);

        OS << Name << " @ depth " << Depth << "\n";

        if (ArgDefBeginsWith)
        {
            printIndent(OS, indent);
            OS << Name << " begins with arg " << ArgDefBeginsWith->Name << "\n";
        }

        if (ArgDefEndsWith)
        {
            printIndent(OS, indent);
            OS << Name << " ends with arg " << ArgDefEndsWith->Name << "\n";
        }

        for (auto Child : Children)
            Child->dumpMacroInfo(OS, indent + 1);
    }

    void MacroExpansionNode::dumpASTInfo(
        llvm::raw_fd_ostream &OS,
        clang::SourceManager &SM,
        const clang::LangOptions &LO)
    {
        OS << "Top level expansion of " << Name << "\n";

        OS << "AST roots: \n";
        for (auto &&Root : ASTRoots)
            Root.dump();

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

    std::set<MacroExpansionNode *> MacroExpansionNode::getDescendants()
    {
        std::set<MacroExpansionNode *> Desc;
        // Collect descendants using BFS
        std::queue<MacroExpansionNode *> Q;
        for (auto &&Child : Children)
            Q.push(Child);

        while (!Q.empty())
        {
            auto Cur = Q.front();
            Q.pop();
            Desc.insert(Cur);
            for (auto &&Child : Cur->Children)
                Q.push(Child);
        }

        return Desc;
    }

} // namespace cpp2c
