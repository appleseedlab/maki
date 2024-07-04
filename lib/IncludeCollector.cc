#include "IncludeCollector.hh"
#include <clang/Basic/FileEntry.h>
#include <clang/Basic/Module.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Lex/Token.h>
#include <llvm/ADT/StringRef.h>

namespace maki {
void IncludeCollector::InclusionDirective(
    clang::SourceLocation HashLoc, const clang::Token &IncludeTok,
    llvm::StringRef FileName, bool IsAngled,
    clang::CharSourceRange FilenameRange, clang::OptionalFileEntryRef File,
    llvm::StringRef SearchPath, llvm::StringRef RelativePath,
    const clang::Module *Imported, clang::SrcMgr::CharacteristicKind FileType) {
    IncludeEntriesLocs.emplace_back(File, HashLoc);
}

} // namespace maki
