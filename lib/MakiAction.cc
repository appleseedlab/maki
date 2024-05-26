#include "MakiAction.hh"
#include "MakiASTConsumer.hh"
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Frontend/FrontendPluginRegistry.h>
#include <llvm-17/llvm/ADT/StringRef.h>
#include <llvm-17/llvm/Support/raw_ostream.h>
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
