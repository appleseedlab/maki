#pragma once

#include <clang/AST/ASTContext.h>
#include <clang/Basic/LangOptions.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Lex/MacroInfo.h>
#include <clang/Lex/PPCallbacks.h>
#include <clang/Lex/Token.h>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace maki {
class DefinitionInfoCollector : public clang::PPCallbacks {
private:
    clang::SourceManager &SM;
    const clang::LangOptions &LO;

public:
    std::vector<std::pair<std::string, const clang::MacroDirective *>>
        MacroNamesDefinitions;
    std::set<std::string> InspectedMacroNames;

    DefinitionInfoCollector(clang::ASTContext &Ctx);

    void MacroDefined(const clang::Token &MacroNameTok,
                      const clang::MacroDirective *MD) override;

    void MacroUndefined(const clang::Token &MacroNameTok,
                        const clang::MacroDefinition &MD,
                        const clang::MacroDirective *Undef) override;

    void Defined(const clang::Token &MacroNameTok,
                 const clang::MacroDefinition &MD,
                 clang::SourceRange Range) override;

    void Ifdef(clang::SourceLocation Loc, const clang::Token &MacroNameTok,
               const clang::MacroDefinition &MD) override;

    void Ifndef(clang::SourceLocation Loc, const clang::Token &MacroNameTok,
                const clang::MacroDefinition &MD) override;
};
} // namespace maki