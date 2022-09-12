#pragma once

#include <clang/Frontend/FrontendPluginRegistry.h>

namespace cpp2c
{
    class Cpp2CAction : public clang::PluginASTAction
    {

    protected:
        std::unique_ptr<clang::ASTConsumer>
        CreateASTConsumer(clang::CompilerInstance &CI,
                          llvm::StringRef InFile) override;

        bool ParseArgs(const clang::CompilerInstance &CI,
                  const std::vector<std::string> &arg) override;

        clang::PluginASTAction::ActionType getActionType() override;
    };

} // namespace cpp2c
