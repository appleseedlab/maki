#pragma once

#include "DefinitionInfoCollector.hh"
#include "IncludeCollector.hh"
#include "MacroForest.hh"
#include "MakiFlags.hh"
#include <clang/AST/ASTConsumer.h>
#include <clang/AST/ASTContext.h>
#include <clang/AST/Stmt.h>
#include <clang/Frontend/CompilerInstance.h>
#include <llvm-17/llvm/Support/Casting.h>

namespace maki {
class MakiASTConsumer : public clang::ASTConsumer {
private:
    maki::MacroForest *MF;
    maki::IncludeCollector *IC;
    maki::DefinitionInfoCollector *DC;
    MakiFlags Flags;

public:
    MakiASTConsumer(clang::CompilerInstance &CI, MakiFlags flags);
    void HandleTranslationUnit(clang::ASTContext &Ctx) override;
};

template <typename T>
inline std::function<bool(const clang::Stmt *)> stmtIsA() {
    return [](const clang::Stmt *ST) { return llvm::isa_and_nonnull<T>(ST); };
}
} // namespace maki
