#pragma once

#include <clang/Basic/SourceLocation.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Lex/MacroInfo.h>
#include <string>
#include <utility>

namespace maki {

struct MakiFlags {
    bool ProcessBuiltinMacros = true;
    bool ProcessMacrosInSystemHeaders = true;
    bool ProcessMacrosAtInvalidLocations = true;
};

std::pair<bool, std::string> tryGetFullSourceLoc(clang::SourceManager &SM,
                                                 clang::SourceLocation L);

// Whether or not to process the given macro definition based on the given
// flags.
bool shouldSkipMacroDefinition(clang::SourceManager &SM, MakiFlags Flags,
                               const clang::MacroInfo *MI);

// Whether or not to process the invocation of the given macro at the given
// location based on the given flags.
bool shouldSkipMacroInvocation(clang::SourceManager &SM, MakiFlags Flags,
                               const clang::MacroInfo *MI,
                               clang::SourceLocation Location);

} // namespace maki
