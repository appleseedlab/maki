#include "DefinitionInfoCollector.hh"

namespace cpp2c
{

    DefinitionInfoCollector::DefinitionInfoCollector(clang::ASTContext &Ctx)
        : SM(Ctx.getSourceManager()), LO(Ctx.getLangOpts()) {}

    void DefinitionInfoCollector::MacroDefined(
        const clang::Token &MacroNameTok,
        const clang::MacroDirective *MD)
    {
        std::string Name = clang::Lexer::getSpelling(MacroNameTok, SM, LO);
        MacroNamesDefinitions.push_back({Name, MD});
    }

    void DefinitionInfoCollector::MacroUndefined(
        const clang::Token &MacroNameTok,
        const clang::MacroDefinition &MD,
        const clang::MacroDirective *Undef)
    {
        auto Name = clang::Lexer::getSpelling(MacroNameTok, SM, LO);
        InspectedMacroNames.insert(Name);
    }

    void DefinitionInfoCollector::Defined(
        const clang::Token &MacroNameTok,
        const clang::MacroDefinition &MD,
        clang::SourceRange Range)
    {
        auto Name = clang::Lexer::getSpelling(MacroNameTok, SM, LO);
        InspectedMacroNames.insert(Name);
    }

    void DefinitionInfoCollector::Ifdef(
        clang::SourceLocation Loc,
        const clang::Token &MacroNameTok,
        const clang::MacroDefinition &MD)
    {
        auto Name = clang::Lexer::getSpelling(MacroNameTok, SM, LO);
        InspectedMacroNames.insert(Name);
    }

    void DefinitionInfoCollector::Ifndef(
        clang::SourceLocation Loc,
        const clang::Token &MacroNameTok,
        const clang::MacroDefinition &MD)
    {
        auto Name = clang::Lexer::getSpelling(MacroNameTok, SM, LO);
        InspectedMacroNames.insert(Name);
    }

} // namespace cpp2c
