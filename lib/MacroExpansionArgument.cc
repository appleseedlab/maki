#include "MacroExpansionArgument.hh"
#include <clang/Basic/LangOptions.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Basic/TokenKinds.h>
#include <clang/Lex/Lexer.h>
#include <llvm-17/llvm/Support/raw_ostream.h>

namespace maki {
void MacroExpansionArgument::dumpASTInfo(llvm::raw_fd_ostream &OS,
                                         clang::SourceManager &SM,
                                         const clang::LangOptions &LO) {
    OS << "Argument " << Name << " tokens:\n";
    for (auto T : Tokens) {
        OS << clang::Lexer::getSpelling(T, SM, LO) << " ("
           << clang::tok::getTokenName(T.getKind()) << ")"
           << " @ ";
        T.getLocation().dump(SM);
    }

    llvm::errs() << "Aligned roots for argument " << Name << ":\n";
    if (!AlignedRoots.empty()) {
        for (auto R : AlignedRoots) {
            R.dump();
        }
    } else {
        llvm::errs() << "None\n";
    }
}

} // namespace maki
