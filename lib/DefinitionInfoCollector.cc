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

void DefinitionInfoCollector::CollectMacroNameToken(const clang::Token &Token) {
    auto Name = clang::Lexer::getSpelling(Token, SM, LO);
    InspectedMacroNames.insert(std::move(Name));
}

void DefinitionInfoCollector::MacroDefined(const clang::Token &MacroNameTok,
                                           const clang::MacroDirective *MD) {
    std::string Name = clang::Lexer::getSpelling(MacroNameTok, SM, LO);
    MacroNamesDefinitions.push_back({ Name, MD });
}

void DefinitionInfoCollector::MacroUndefined(
    const clang::Token &MacroNameTok, const clang::MacroDefinition &MD,
    const clang::MacroDirective *Undef) {
    CollectMacroNameToken(MacroNameTok);
}

void DefinitionInfoCollector::Defined(const clang::Token &MacroNameTok,
                                      const clang::MacroDefinition &MD,
                                      clang::SourceRange Range) {
    CollectMacroNameToken(MacroNameTok);
}

void DefinitionInfoCollector::Ifdef(clang::SourceLocation Loc,
                                    const clang::Token &MacroNameTok,
                                    const clang::MacroDefinition &MD) {
    CollectMacroNameToken(MacroNameTok);
}

// NOTE(Brent): This only visits branches that are taken.
void DefinitionInfoCollector::Elifdef(clang::SourceLocation Loc,
                                      const clang::Token &MacroNameTok,
                                      const clang::MacroDefinition &MD) {
    CollectMacroNameToken(MacroNameTok);
}

void DefinitionInfoCollector::Ifndef(clang::SourceLocation Loc,
                                     const clang::Token &MacroNameTok,
                                     const clang::MacroDefinition &MD) {
    CollectMacroNameToken(MacroNameTok);
}

// NOTE(Brent): This only visits branches that are taken.
void DefinitionInfoCollector::Elifndef(clang::SourceLocation Loc,
                                       const clang::Token &MacroNameTok,
                                       const clang::MacroDefinition &MD) {
    CollectMacroNameToken(MacroNameTok);
}
} // namespace maki
