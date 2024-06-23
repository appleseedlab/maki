#include "MakiFlags.hh"
#include "SourceLocationUtils.hh"
#include <clang/Basic/SourceManager.h>
#include <clang/Lex/MacroInfo.h>
#include <tuple>

namespace maki {

bool shouldSkipMacroDefinition(clang::SourceManager &SM, MakiFlags Flags,
                               const clang::MacroInfo *MI) {
    if (!MI) {
        return true;
    }
    if (!Flags.ProcessBuiltinMacros && MI->isBuiltinMacro()) {
        return true;
    }
    auto DefinitionLocation = MI->getDefinitionLoc();
    if (!Flags.ProcessMacrosInSystemHeaders &&
        SM.isInSystemHeader(DefinitionLocation)) {
        return true;
    }
    if (!Flags.ProcessMacrosAtInvalidLocations) {
        bool Valid = false;
        std::tie(Valid, std::ignore) =
            tryGetFullSourceLoc(SM, DefinitionLocation);
        if (!Valid) {
            return true;
        }
    }
    return false;
}

bool shouldSkipMacroInvocation(clang::SourceManager &SM, MakiFlags Flags,
                               const clang::MacroInfo *MI,
                               clang::SourceLocation Location) {
    if (!MI) {
        return true;
    }
    if (shouldSkipMacroDefinition(SM, Flags, MI)) {
        return true;
    }
    if (!Flags.ProcessMacrosInSystemHeaders && SM.isInSystemHeader(Location)) {
        return true;
    }
    if (!Flags.ProcessMacrosAtInvalidLocations) {
        bool Valid = false;
        std::tie(Valid, std::ignore) = tryGetFullSourceLoc(SM, Location);
        if (!Valid) {
            return true;
        }
    }
    return false;
}

} // namespace maki