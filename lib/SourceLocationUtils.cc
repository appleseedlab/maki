#include "SourceLocationUtils.hh"
#include <clang/Basic/SourceLocation.h>
#include <clang/Basic/SourceManager.h>
#include <string>
#include <utility>

namespace maki {
std::pair<bool, std::string>
tryGetFullSourceLoc(clang::SourceManager &SM, clang::SourceLocation Location) {
    if (Location.isInvalid()) {
        return { false, "Invalid SLoc" };
    }
    auto FID = SM.getFileID(Location);
    if (FID.isInvalid()) {
        return { false, "Invalid file ID" };
    }
    auto FE = SM.getFileEntryForID(FID);
    if (nullptr == FE) {
        return { false, "File without FileEntry" };
    }
    auto Name = FE->tryGetRealPathName();
    if (Name.empty()) {
        return { false, "Nameless file" };
    }
    auto FLoc = SM.getFileLoc(Location);
    if (FLoc.isInvalid()) {
        return { false, "Invalid File SLoc" };
    }

    // Use getSpellingLineNumber instead of presumed location to ignore #line directives
    auto LineNum = SM.getSpellingLineNumber(FLoc);
    auto ColNum = SM.getSpellingColumnNumber(FLoc);

    return { true, Name.str() + ":" + std::to_string(LineNum) + ":" + std::to_string(ColNum) };
}
}
