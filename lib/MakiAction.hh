#pragma once

#include "MakiFlags.hh"
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
    MakiFlags Flags;

    std::unique_ptr<clang::ASTConsumer>
    CreateASTConsumer(clang::CompilerInstance &CI,
                      llvm::StringRef InFile) override;

    bool ParseArgs(const clang::CompilerInstance &CI,
                   const std::vector<std::string> &args) override;

    clang::PluginASTAction::ActionType getActionType() override;
};

} // namespace maki
