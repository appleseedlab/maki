#include "Cpp2CASTConsumer.hh"
#include "ExpansionArgumentMatchHandler.hh"
#include "ExpansionMatchHandler.hh"
#include "ExpansionMatcher.hh"
#include "DeclStmtTypeLoc.hh"

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

    DeclStmtTypeLoc *firstASTRootSpelledInRange(
        clang::SourceManager &SM,
        std::vector<DeclStmtTypeLoc> &ASTRoots,
        clang::SourceRange Range)
    {

        // TODO: Maybe we should check that spelling location of end loc
        // is in range instead of begin loc?
        for (auto &&R : ASTRoots)
            if (Range.fullyContains(
                    SM.getSpellingLoc(R.getSourceRange().getBegin())))
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
        for (auto TLE : MF->TopLevelExpansions)
        {
            // TLE->dump(llvm::errs());
            using namespace clang::ast_matchers;

            //// Find potential AST roots of the entire expansion

            // Match stmts
            MATCH_EXPANSION_ROOTS_OF(stmt, TLE);

            // Match decls
            MATCH_EXPANSION_ROOTS_OF(decl, TLE);

            // Match type locs
            MATCH_EXPANSION_ROOTS_OF(typeLoc, TLE);

            llvm::errs() << "Top level expansion of "
                         << TLE->Name.str()
                         << "\n";
            // llvm::errs() << "AST roots:\n";
            // if (!TLE->ASTRoots.empty())
            //     for (auto R : TLE->ASTRoots)
            //         R.dump();
            // else
            //     llvm::errs() << "None\n";
            TLE->AlignedRoot = firstASTRootExpandsToAlignWithRange(SM,
                                                                   TLE->ASTRoots,
                                                                   TLE->SpellingRange);
            llvm::errs() << "Aligned root: \n";
            if (TLE->AlignedRoot)
                TLE->AlignedRoot->dump();
            else
                llvm::errs() << "None\n";

            //// Find potential AST roots of the expansion's arguments
            if (!TLE->Arguments.empty())
            {
                for (auto &&Arg : TLE->Arguments)
                {

                    // Match stmts
                    MATCH_ARGUMENT_ROOTS_OF(stmt, (&Arg));

                    // Match decls
                    MATCH_ARGUMENT_ROOTS_OF(decl, (&Arg));

                    // Match type locs
                    MATCH_ARGUMENT_ROOTS_OF(typeLoc, (&Arg));

                    llvm::errs() << "Number of AST roots for argument "
                                 << Arg.Name << ": "
                                 << std::to_string(Arg.ASTRoots.size())
                                 << "\n";

                    if (!Arg.TokenRanges.empty())
                    {
                        clang::SourceRange ArgRange(
                            Arg.TokenRanges.front().getBegin(),
                            Arg.TokenRanges.back().getEnd());

                        // llvm::errs() << "Arg range: ";
                        // ArgRange.dump(SM);

                        Arg.AlignedRoot =
                            firstASTRootSpelledInRange(SM,
                                                       Arg.ASTRoots,
                                                       ArgRange);
                    }
                    // Try to find an AST node aligned with this argument
                    llvm::errs() << "Aligned root for argument "
                                 << Arg.Name
                                 << ":\n";
                    if (Arg.AlignedRoot)
                        Arg.AlignedRoot->dump();
                    else
                        llvm::errs() << "None\n";
                }
            }
            else
                llvm::errs() << "No arguments\n";
        }
    }
} // namespace cpp2c
