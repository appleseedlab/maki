#include "MakiAction.hh"
#include "MakiASTConsumer.hh"

namespace maki {
std::unique_ptr<clang::ASTConsumer>
MakiAction::CreateASTConsumer(clang::CompilerInstance &CI,
                              llvm::StringRef InFile) {
    return std::make_unique<maki::MakiASTConsumer>(CI);
}

bool MakiAction::ParseArgs(const clang::CompilerInstance &CI,
                           const std::vector<std::string> &arg) {
    return true;
}

clang::PluginASTAction::ActionType MakiAction::getActionType() {
    return clang::PluginASTAction::ActionType::AddBeforeMainAction;
}

static clang::FrontendPluginRegistry::Add<MakiAction>
    X("maki", "Analyze properties of macro invocations");
} // namespace maki