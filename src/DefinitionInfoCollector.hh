#pragma once

#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Lex/PPCallbacks.h"
#include "clang/Lex/Token.h"
#include "clang/Lex/MacroInfo.h"

#include <vector>
#include <set>
#include <utility>

namespace cpp2c
{
    class DefinitionInfoCollector : public clang::PPCallbacks
    {

        // Need to keep track of
        // - All unique macro definitions
        // - Names of all macros appearing in undefs, defined,
        //   ifdef, and ifndef

    public:
        std::vector<std::pair<std::string, const clang::MacroDirective *>>
            MacroNamesDefinitions;
        std::set<std::string> InspectedMacroNames;

        void MacroDefined(const clang::Token &MacroNameTok,
                          const clang::MacroDirective *MD) override;

        void MacroUndefined(const clang::Token &MacroNameTok,
                            const clang::MacroDefinition &MD,
                            const clang::MacroDirective *Undef) override;

        void Defined(const clang::Token &MacroNameTok,
                     const clang::MacroDefinition &MD,
                     clang::SourceRange Range) override;

        void Ifdef(clang::SourceLocation Loc,
                   const clang::Token &MacroNameTok,
                   const clang::MacroDefinition &MD) override;

        void Ifndef(clang::SourceLocation Loc,
                    const clang::Token &MacroNameTok,
                    const clang::MacroDefinition &MD) override;
    };
} // namespace cpp2c