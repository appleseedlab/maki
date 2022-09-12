#include "Cpp2CAction.hh"
#include "Cpp2CASTConsumer.hh"

namespace cpp2c
{
    std::unique_ptr<clang::ASTConsumer>
    Cpp2CAction::CreateASTConsumer(clang::CompilerInstance &CI,
                                   llvm::StringRef InFile)
    {
        return std::make_unique<cpp2c::Cpp2CASTConsumer>(CI);
    }

    bool Cpp2CAction::ParseArgs(const clang::CompilerInstance &CI,
                                const std::vector<std::string> &arg)
    {
        return true;
    }

    clang::PluginASTAction::ActionType Cpp2CAction::getActionType()
    {
        return clang::PluginASTAction::ActionType::AddBeforeMainAction;
    }

    static clang::FrontendPluginRegistry::Add<Cpp2CAction>
        X("macro-types", "Extract macro typing information");
} // namespace cpp2c
