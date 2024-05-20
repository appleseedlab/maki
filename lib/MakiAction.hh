#pragma once

#include <clang/AST/ASTConsumer.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>
#include <llvm-17/llvm/ADT/StringRef.h>
#include <memory>
#include <string>
#include <vector>

namespace maki {
class MakiAction : public clang::PluginASTAction {
protected:
    std::unique_ptr<clang::ASTConsumer>
    CreateASTConsumer(clang::CompilerInstance &CI,
                      llvm::StringRef InFile) override;

    bool ParseArgs(const clang::CompilerInstance &CI,
                   const std::vector<std::string> &arg) override;

    clang::PluginASTAction::ActionType getActionType() override;
};

} // namespace maki
