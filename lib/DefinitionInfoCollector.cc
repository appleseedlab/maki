#include "DefinitionInfoCollector.hh"
#include "MakiFlags.hh"
#include <clang/AST/ASTContext.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Lex/Lexer.h>
#include <clang/Lex/MacroInfo.h>
#include <clang/Lex/Token.h>

namespace maki {
DefinitionInfoCollector::DefinitionInfoCollector(clang::ASTContext &Ctx,
                                                 MakiFlags Flags)
    : SM(Ctx.getSourceManager())
    , LO(Ctx.getLangOpts())
    , Flags(Flags) {
}

void DefinitionInfoCollector::CollectMacroName(
    const clang::Token &Token, const clang::MacroDefinition &MD) {
    // Only skip macro definitions which we are sure exist.
    if (MD && shouldSkipMacroDefinition(SM, Flags, MD)) {
        return;
    }
    auto Name = clang::Lexer::getSpelling(Token, SM, LO);
    InspectedMacroNames.insert(std::move(Name));
}

void DefinitionInfoCollector::MacroDefined(const clang::Token &MacroNameTok,
                                           const clang::MacroDirective *MD) {
    if (MD) {
        auto MI = MD->getMacroInfo();
        if (MI && shouldSkipMacroDefinition(SM, Flags, MI)) {
            return;
        }
    }
    std::string Name = clang::Lexer::getSpelling(MacroNameTok, SM, LO);
    MacroNamesDefinitions.push_back({ Name, MD });
}

void DefinitionInfoCollector::MacroUndefined(
    const clang::Token &MacroNameTok, const clang::MacroDefinition &MD,
    const clang::MacroDirective *Undef) {
    CollectMacroName(MacroNameTok, MD);
}

void DefinitionInfoCollector::Defined(const clang::Token &MacroNameTok,
                                      const clang::MacroDefinition &MD,
                                      clang::SourceRange Range) {
    CollectMacroName(MacroNameTok, MD);
}

void DefinitionInfoCollector::Ifdef(clang::SourceLocation Loc,
                                    const clang::Token &MacroNameTok,
                                    const clang::MacroDefinition &MD) {
    CollectMacroName(MacroNameTok, MD);
}

// NOTE(Brent): This only visits branches that are taken.
void DefinitionInfoCollector::Elifdef(clang::SourceLocation Loc,
                                      const clang::Token &MacroNameTok,
                                      const clang::MacroDefinition &MD) {
    CollectMacroName(MacroNameTok, MD);
}

void DefinitionInfoCollector::Ifndef(clang::SourceLocation Loc,
                                     const clang::Token &MacroNameTok,
                                     const clang::MacroDefinition &MD) {
    CollectMacroName(MacroNameTok, MD);
}

// NOTE(Brent): This only visits branches that are taken.
void DefinitionInfoCollector::Elifndef(clang::SourceLocation Loc,
                                       const clang::Token &MacroNameTok,
                                       const clang::MacroDefinition &MD) {
    CollectMacroName(MacroNameTok, MD);
}
} // namespace maki
