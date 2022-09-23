#include "MacroExpansionArgument.hh"

#include "clang/Lex/Lexer.h"
#include "clang/Basic/SourceManager.h"

namespace cpp2c
{
    void MacroExpansionArgument::dumpASTInfo(
        llvm::raw_fd_ostream &OS,
        clang::SourceManager &SM,
        const clang::LangOptions &LO)
    {
        OS << "Argument " << Name << " tokens:\n";
        for (auto T : Tokens)
        {
            OS << clang::Lexer::getSpelling(T, SM, LO)
               << " (" << clang::tok::getTokenName(T.getKind()) << ")"
               << " @ ";
            T.getLocation().dump(SM);
        }

        llvm::errs() << "Aligned roots for argument "
                     << Name
                     << ":\n";
        if (!AlignedRoots.empty())
            for (auto R : AlignedRoots)
                R.dump();
        else
            llvm::errs() << "None\n";
    }

} // namespace cpp2c
