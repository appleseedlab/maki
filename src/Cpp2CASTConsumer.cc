#include "Cpp2CASTConsumer.hh"
#include "ExpansionMatchHandler.hh"
#include "ExpansionMatcher.hh"
#include "DeclStmtTypeLoc.hh"

#include "clang/Lex/Lexer.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

namespace cpp2c
{

    DeclStmtTypeLoc *firstASTRootExpandsToAlignWithRange(
        clang::SourceManager &SM,
        std::vector<DeclStmtTypeLoc> &ASTRoots,
        clang::SourceRange Range)
    {

        // TODO: Maybe instead we should align the expansion with the root
        // that has the most inclusive range?
        for (auto &&R : ASTRoots)
            if (Range == SM.getExpansionRange(R.getSourceRange()).getAsRange())
                return &R;
        return nullptr;
    }

    Cpp2CASTConsumer::Cpp2CASTConsumer(clang::CompilerInstance &CI)
    {
        clang::Preprocessor &PP = CI.getPreprocessor();
        clang::ASTContext &Ctx = CI.getASTContext();

        MF = new cpp2c::MacroForest(PP, Ctx);

        PP.addPPCallbacks(std::unique_ptr<cpp2c::MacroForest>(MF));
    }

    void Cpp2CASTConsumer::HandleTranslationUnit(clang::ASTContext &Ctx)
    {
        auto &SM = Ctx.getSourceManager();
        const auto &LO = Ctx.getLangOpts();
        for (auto TLE : MF->TopLevelExpansions)
        {
            using namespace clang::ast_matchers;

            //// Find potential AST roots of the entire expansion

            // Match stmts
            MATCH_EXPANSION_ROOTS_OF(stmt, TLE);

            // Match decls
            MATCH_EXPANSION_ROOTS_OF(decl, TLE);

            // Match type locs
            MATCH_EXPANSION_ROOTS_OF(typeLoc, TLE);

            TLE->AlignedRoot = firstASTRootExpandsToAlignWithRange(
                SM,
                TLE->ASTRoots,
                TLE->SpellingRange);

            //// Find AST roots aligned with each of the expansion's arguments
            for (auto &&Arg : TLE->Arguments)
            {
                // Match stmts
                MATCH_ARGUMENT_ROOTS_OF(stmt, (&Arg));

                // Match decls
                MATCH_ARGUMENT_ROOTS_OF(decl, (&Arg));

                // Match type locs
                MATCH_ARGUMENT_ROOTS_OF(typeLoc, (&Arg));
            }

            TLE->dumpASTInfo(llvm::errs(), SM, LO);
        }
    }
} // namespace cpp2c
