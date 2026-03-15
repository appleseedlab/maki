#pragma once

#include "MakiFlags.hh"
#include <clang/AST/ASTContext.h>
#include <clang/Basic/LangOptions.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Lex/MacroInfo.h>
#include <clang/Lex/PPCallbacks.h>
#include <clang/Lex/Preprocessor.h>
#include <llvm-17/llvm/ADT/StringRef.h>
#include <map>
#include <set>

namespace maki {
class InvokedDefinitionInfoCollector : public clang::PPCallbacks {
private:
    clang::SourceManager &SM;
    const clang::LangOptions &LO;
    MakiFlags Flags;

public:
    std::map<llvm::StringRef, std::set<clang::MacroInfo *>> MacroNamesInfos;

    InvokedDefinitionInfoCollector(clang::ASTContext &Ctx, MakiFlags Flags);

    void MacroExpands(const clang::Token &MacroNameTok,
                      const clang::MacroDefinition &MD,
                      clang::SourceRange Range,
                      const clang::MacroArgs *Args) override;
};
} // namespace maki
