#pragma once

#include "MacroForest.hh"
#include "IncludeCollector.hh"
#include "DefinitionInfoCollector.hh"

#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/CompilerInstance.h"

namespace cpp2c
{
    class Cpp2CASTConsumer : public clang::ASTConsumer
    {
    private:
        cpp2c::MacroForest *MF;
        cpp2c::IncludeCollector *IC;
        cpp2c::DefinitionInfoCollector *DC;

    public:
        Cpp2CASTConsumer(clang::CompilerInstance &CI);
        void HandleTranslationUnit(clang::ASTContext &Ctx) override;
    };

    template <typename T>
    inline std::function<bool(const clang::Stmt *)> stmtIsA()
    {
        return [](const clang::Stmt *ST)
        { return llvm::isa_and_nonnull<T>(ST); };
    }
} // namespace cpp2c
