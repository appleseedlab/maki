#include "InvokedDefinitionInfoCollector.hh"
#include "MakiFlags.hh"
#include <clang/AST/ASTContext.h>

namespace maki {
InvokedDefinitionInfoCollector::InvokedDefinitionInfoCollector(
    clang::ASTContext &Ctx, MakiFlags Flags)
    : SM(Ctx.getSourceManager())
    , LO(Ctx.getLangOpts()) {
}

void InvokedDefinitionInfoCollector::MacroExpands(
    const clang::Token &MacroNameTok, const clang::MacroDefinition &MD,
    clang::SourceRange Range, const clang::MacroArgs *Args) {
    auto BeginSpellingLocation = SM.getSpellingLoc(Range.getBegin());
    if (shouldSkipMacroDefinition(SM, Flags, MD) ||
        shouldSkipMacroInvocation(SM, Flags, MD, BeginSpellingLocation)) {
        return;
    }

    auto Name = MacroNameTok.getIdentifierInfo()->getName();
    MacroNamesInfos[Name].insert(MD.getMacroInfo());
}
} // namespace maki
