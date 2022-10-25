#include "DefinitionInfoCollector.hh"

namespace cpp2c
{

    void DefinitionInfoCollector::MacroDefined(
        const clang::Token &MacroNameTok,
        const clang::MacroDirective *MD)
    {
        std::string Name(MacroNameTok.getName());
        MacroNamesDefinitions.push_back({Name, MD});
    }

    void DefinitionInfoCollector::MacroUndefined(
        const clang::Token &MacroNameTok,
        const clang::MacroDefinition &MD,
        const clang::MacroDirective *Undef)
    {
        InspectedMacroNames.insert(MacroNameTok.getName());
    }

    void DefinitionInfoCollector::Defined(
        const clang::Token &MacroNameTok,
        const clang::MacroDefinition &MD,
        clang::SourceRange Range)
    {
        InspectedMacroNames.insert(MacroNameTok.getName());
    }

    void DefinitionInfoCollector::Ifdef(
        clang::SourceLocation Loc,
        const clang::Token &MacroNameTok,
        const clang::MacroDefinition &MD)
    {
        InspectedMacroNames.insert(MacroNameTok.getName());
    }

    void DefinitionInfoCollector::Ifndef(
        clang::SourceLocation Loc,
        const clang::Token &MacroNameTok,
        const clang::MacroDefinition &MD)
    {
        InspectedMacroNames.insert(MacroNameTok.getName());
    }

} // namespace cpp2c
