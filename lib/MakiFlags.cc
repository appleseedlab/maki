#include "MakiFlags.hh"
#include "SourceLocationUtils.hh"
#include <clang/Basic/SourceManager.h>
#include <clang/Lex/MacroInfo.h>

namespace maki {

bool shouldSkipMacroDefinition(clang::SourceManager &SM, MakiFlags Flags,
                               const clang::MacroInfo *MI) {
    return (!Flags.ProcessBuiltinMacros && MI->isBuiltinMacro()) ||
           (!Flags.ProcessMacrosInSystemHeaders &&
            SM.isInSystemHeader(MI->getDefinitionLoc())) ||
           (!Flags.ProcessMacrosAtInvalidLocations &&
            !tryGetFullSourceLoc(SM, MI->getDefinitionLoc()).first);
}

bool shouldSkipMacroInvocation(clang::SourceManager &SM, MakiFlags Flags,
                               const clang::MacroInfo *MI,
                               clang::SourceLocation Location) {
    return shouldSkipMacroDefinition(SM, Flags, MI) ||
           (!Flags.ProcessMacrosInSystemHeaders &&
            SM.isInSystemHeader(Location)) ||
           (!Flags.ProcessMacrosAtInvalidLocations &&
            !tryGetFullSourceLoc(SM, Location).first);
}

} // namespace maki