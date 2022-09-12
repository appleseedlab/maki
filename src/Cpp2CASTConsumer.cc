#include "Cpp2CASTConsumer.hh"

#include "clang/Lex/Preprocessor.h"

namespace cpp2c
{
    Cpp2CASTConsumer::Cpp2CASTConsumer(clang::CompilerInstance &CI)
    {
        clang::Preprocessor &PP = CI.getPreprocessor();
        clang::ASTContext &Ctx = CI.getASTContext();

        MF = new cpp2c::MacroForest(PP, Ctx);

        PP.addPPCallbacks(std::unique_ptr<cpp2c::MacroForest>(MF));
    }

    void Cpp2CASTConsumer::HandleTranslationUnit(clang::ASTContext &Ctx)
    {
        for (auto TopLevelExpansion : MF->TopLevelExpansions)
        {
            TopLevelExpansion->dump(llvm::errs());
        }
    }
} // namespace cpp2c
