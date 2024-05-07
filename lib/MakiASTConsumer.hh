#pragma once

#include "DefinitionInfoCollector.hh"
#include "IncludeCollector.hh"
#include "MacroForest.hh"

#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/CompilerInstance.h"

namespace maki {
class MakiASTConsumer : public clang::ASTConsumer {
private:
    maki::MacroForest *MF;
    maki::IncludeCollector *IC;
    maki::DefinitionInfoCollector *DC;

public:
    MakiASTConsumer(clang::CompilerInstance &CI);
    void HandleTranslationUnit(clang::ASTContext &Ctx) override;
};

template <typename T>
inline std::function<bool(const clang::Stmt *)> stmtIsA() {
    return [](const clang::Stmt *ST) { return llvm::isa_and_nonnull<T>(ST); };
}
} // namespace maki
