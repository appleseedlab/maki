#pragma once

#include "MacroForest.hh"

#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/CompilerInstance.h"

namespace cpp2c
{
    class Cpp2CASTConsumer : public clang::ASTConsumer
    {
    private:
        cpp2c::MacroForest *MF;

    public:
        Cpp2CASTConsumer(clang::CompilerInstance &CI);
        void HandleTranslationUnit(clang::ASTContext &Ctx) override;
    };

} // namespace cpp2c
