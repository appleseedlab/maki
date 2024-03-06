#pragma once

#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/FileEntry.h"
#include "clang/Basic/Module.h"
#include "clang/Lex/PPCallbacks.h"
#include "clang/Lex/Token.h"

#include "llvm/ADT/StringRef.h"

#include <vector>
#include <utility>

namespace cpp2c
{
    class IncludeCollector : public clang::PPCallbacks
    {
    public:
        std::vector<std::pair<clang::OptionalFileEntryRef, clang::SourceLocation>>
            IncludeEntriesLocs;

        void InclusionDirective(
            clang::SourceLocation HashLoc,
            const clang::Token &IncludeTok,
            llvm::StringRef FileName,
            bool IsAngled,
            clang::CharSourceRange FilenameRange,
            clang::OptionalFileEntryRef File,
            llvm::StringRef SearchPath,
            llvm::StringRef RelativePath,
            const clang::Module *Imported,
            clang::SrcMgr::CharacteristicKind FileType) override;
    };
} // namespace cpp2c 
