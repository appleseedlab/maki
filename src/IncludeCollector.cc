#include "IncludeCollector.hh"

namespace cpp2c
{
    void IncludeCollector::InclusionDirective(
        clang::SourceLocation HashLoc,
        const clang::Token &IncludeTok,
        llvm::StringRef FileName,
        bool IsAngled,
        clang::CharSourceRange FilenameRange,
        const clang::FileEntry *File,
        llvm::StringRef SearchPath,
        llvm::StringRef RelativePath,
        const clang::Module *Imported,
        clang::SrcMgr::CharacteristicKind FileType)
    {
        IncludeEntriesLocs.emplace_back(File, HashLoc);
    }

} // namespace cpp2c
