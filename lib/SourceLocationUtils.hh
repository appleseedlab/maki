#pragma once

#include <clang/Basic/SourceLocation.h>
#include <clang/Basic/SourceManager.h>
#include <string>
#include <utility>

namespace maki {

// Tries to get the full real path and line + column number for a given source
// location. First element is whether the operation was successful, the second
// is the error if not and the full path if successful.
std::pair<bool, std::string>
tryGetFullSourceLoc(clang::SourceManager &SM, clang::SourceLocation Location);

}