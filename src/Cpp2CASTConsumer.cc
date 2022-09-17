#include "Cpp2CASTConsumer.hh"
#include "ExpansionMatchHandler.hh"
#include "ExpansionMatcher.hh"
#include "DeclStmtTypeLoc.hh"

#include "clang/Lex/Lexer.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

#include <algorithm>
#include <functional>

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

    bool isInTree(
        const clang::Stmt *ST,
        std::function<bool(const clang::Stmt *)> pred)
    {
        if (pred(ST))
            return true;
        for (auto child : ST->children())
            if (isInTree(child, pred))
                return true;
        return false;
    }

    template <typename T>
    inline std::function<bool(const clang::Stmt *)> stmtIsA()
    {
        return [](const clang::Stmt *ST)
        { return clang::isa<T>(ST); };
    }

    inline std::function<bool(const clang::Stmt *)>
    stmtIsBinOp(clang::BinaryOperator::Opcode OC)
    {
        return [OC](const clang::Stmt *ST)
        {
            if (auto BO = clang::dyn_cast<clang::BinaryOperator>(ST))
                return BO->getOpcode() == OC;
            return false;
        };
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

            // TLE->dumpASTInfo(llvm::errs(), SM, LO);
            if (!TLE->AlignedRoot)
                llvm::errs() << "Unaligned body,";
            else
            {
                llvm::errs() << "Aligned body,";
                auto D = TLE->AlignedRoot->D;
                auto ST = TLE->AlignedRoot->ST;
                auto TL = TLE->AlignedRoot->TL;

                if (ST)
                    llvm::errs() << "Stmt,";

                if (llvm::isa_and_nonnull<clang::DoStmt>(ST))
                    llvm::errs() << "DoStmt,";
                else
                {
                    if (isInTree(ST, stmtIsA<clang::ContinueStmt>()))
                        llvm::errs() << "ContinueStmt,";
                    if (isInTree(ST, stmtIsA<clang::BreakStmt>()))
                        llvm::errs() << "BreakStmt,";
                }

                if (isInTree(ST, stmtIsA<clang::ReturnStmt>()))
                    llvm::errs() << "ReturnStmt,";
                if (isInTree(ST, stmtIsA<clang::GotoStmt>()))
                    llvm::errs() << "GotoStmt,";

                if (llvm::isa_and_nonnull<clang::CharacterLiteral>(ST))
                    llvm::errs() << "CharacterLiteral,";
                if (llvm::isa_and_nonnull<clang::IntegerLiteral>(ST))
                    llvm::errs() << "IntegerLiteral,";
                if (llvm::isa_and_nonnull<clang::FloatingLiteral>(ST))
                    llvm::errs() << "FloatingLiteral,";
                if (llvm::isa_and_nonnull<clang::FixedPointLiteral>(ST))
                    llvm::errs() << "FixedPointLiteral,";
                if (llvm::isa_and_nonnull<clang::ImaginaryLiteral>(ST))
                    llvm::errs() << "ImaginaryLiteral,";

                if (llvm::isa_and_nonnull<clang::StringLiteral>(ST))
                    llvm::errs() << "StringLiteral,";
                if (llvm::isa_and_nonnull<clang::CompoundLiteralExpr>(ST))
                    llvm::errs() << "CompoundLiteralExpr,";

                if (isInTree(ST, stmtIsA<clang::ConditionalOperator>()))
                    llvm::errs() << "ConditionalOperator,";
                if (isInTree(ST,
                             stmtIsBinOp(
                                 clang::BinaryOperator::Opcode::BO_LAnd)))
                    llvm::errs() << "BinaryOperator::Opcode::BO_LAnd,";
                if (isInTree(ST,
                             stmtIsBinOp(
                                 clang::BinaryOperator::Opcode::BO_LOr)))
                    llvm::errs() << "BinaryOperator::Opcode::BO_LOr,";

                if (D)
                    llvm::errs() << "Decl,";

                if (TL)
                    llvm::errs() << "TypeLoc,";
            }

            // Check that the number of AST nodes aligned with each argument
            // equals the number of times that argument was expanded
            if (std::all_of(
                    TLE->Arguments.begin(),
                    TLE->Arguments.end(),
                    [](MacroExpansionArgument Arg)
                    { return Arg.AlignedRoots.size() ==
                             Arg.numberOfTimesExpanded; }))
                llvm::errs() << "Aligned arguments,";
            else
                llvm::errs() << "Unaligned argument,";
            llvm::errs() << "\n";
        }
    }
} // namespace cpp2c
