#include "MakiAction.hh"
#include "MakiASTConsumer.hh"
#include "MakiFlags.hh"
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Frontend/FrontendPluginRegistry.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/raw_ostream.h>
#include <string>

namespace maki {
std::unique_ptr<clang::ASTConsumer>
MakiAction::CreateASTConsumer(clang::CompilerInstance &CI,
                              llvm::StringRef InFile) {
    return std::make_unique<maki::MakiASTConsumer>(CI, Flags);
}

bool MakiAction::ParseArgs(const clang::CompilerInstance &CI,
                           const std::vector<std::string> &args) {
    for (const auto &arg : args) {
        std::string OnlyCollectDefinitionInfoArg =
            "--only-collect-definition-info";

        if ("--system-macros" == arg) {
            Flags.ProcessMacrosInSystemHeaders = true;
        } else if ("--no-system-macros" == arg) {
            Flags.ProcessMacrosInSystemHeaders = false;
        } else if ("--builtin-macros" == arg) {
            Flags.ProcessBuiltinMacros = true;
        } else if ("--no-builtin-macros" == arg) {
            Flags.ProcessBuiltinMacros = false;
        } else if ("--invalid-macros" == arg) {
            Flags.ProcessMacrosAtInvalidLocations = true;
        } else if ("--no-invalid-macros" == arg) {
            Flags.ProcessMacrosAtInvalidLocations = false;
        } else if (OnlyCollectDefinitionInfoArg ==
                   arg.substr(0, OnlyCollectDefinitionInfoArg.length())) {
            std::size_t equals_sign_index = arg.find("=");
            if (std::string::npos == equals_sign_index) {
                llvm::errs()
                    << "Error: Expected equals sign followed by 'no', 'all', or 'invoked'\n";
                return false;
            }
            std::string value = arg.substr(
                equals_sign_index + 1, arg.length() - equals_sign_index - 1);
            if ("no" == value) {
                Flags.OnlyCollectDefinitionInfo = NO;
            } else if ("all" == value) {
                Flags.OnlyCollectDefinitionInfo = ALL;
            } else if ("invoked" == value) {
                Flags.OnlyCollectDefinitionInfo = INVOKED;
            } else {
                llvm::errs()
                    << "Error: Expected equals sign followed by 'no', 'all', or 'invoked'\n";
                return false;
            }
        } else {
            llvm::errs() << "Error: Unrecognized argument: " << arg << '\n';
            return false;
        }
    }
    return true;
}

clang::PluginASTAction::ActionType MakiAction::getActionType() {
    return clang::PluginASTAction::ActionType::AddBeforeMainAction;
}

static clang::FrontendPluginRegistry::Add<MakiAction>
    X("maki", "Analyze properties of macro invocations");
} // namespace maki
