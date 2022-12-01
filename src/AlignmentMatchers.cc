#include "AlignmentMatchers.hh"
#include "ExpansionMatchHandler.hh"

namespace cpp2c
{

    void storeChildren(cpp2c::DeclStmtTypeLoc DSTL,
                       std::set<const clang::Stmt *> &MatchedStmts,
                       std::set<const clang::Decl *> &MatchedDecls,
                       std::set<const clang::TypeLoc *> &MatchedTypeLocs)
    {
        if (DSTL.ST)
        {
            std::stack<const clang::Stmt *> Descendants;
            Descendants.push(DSTL.ST);
            while (!Descendants.empty())
            {
                auto Cur = Descendants.top();
                Descendants.pop();
                if (!Cur)
                    continue;

                // llvm::errs() << "Inserting:\n";
                // Cur->dumpColor();
                MatchedStmts.insert(Cur);
                for (auto &&child : Cur->children())
                    if (child)
                        Descendants.push(child);
            }
        }
        else if (DSTL.D)
        {
            // llvm::errs() << "Inserting:\n";
            // DSTL.D->dump();
            MatchedDecls.insert(DSTL.D);
        }
        // else if (DSTL.TL)
        // {
        //     // TODO: Determine why this must be commented out to be able to
        //     //       correctly match TypeLocs
        //     // llvm::errs() << "Inserting:\n";
        // {
        //     auto QT = DSTL.TL->getType();
        //     if (!QT.isNull())
        //         QT.dump();
        //     else
        //         llvm::errs() << "<Null type>\n";
        // }
        //     MatchedTypeLocs.insert(DSTL.TL);
        // }
    }

     void findAlignedASTNodesForExpansion(
        cpp2c::MacroExpansionNode *Exp,
        clang::ASTContext &Ctx)
    {

        using namespace clang::ast_matchers;
        // Find AST nodes aligned with the entire invocation

        // Match stmts
        {
            MatchFinder Finder;
            ExpansionMatchHandler Handler;
            auto Matcher = stmt(unless(anyOf(implicitCastExpr(),
                                             implicitValueInitExpr())),
                                alignsWithExpansion(&Ctx, Exp))
                               .bind("root");
            Finder.addMatcher(Matcher, &Handler);
            Finder.matchAST(Ctx);
            for (auto &&M : Handler.Matches)
                Exp->ASTRoots.push_back(M);
        }

        // Match decls
        {
            MatchFinder Finder;
            ExpansionMatchHandler Handler;
            auto Matcher = decl(alignsWithExpansion(&Ctx, Exp))
                               .bind("root");
            Finder.addMatcher(Matcher, &Handler);
            Finder.matchAST(Ctx);
            for (auto &&M : Handler.Matches)
                Exp->ASTRoots.push_back(M);
        }

        // Match type locs
        {
            MatchFinder Finder;
            ExpansionMatchHandler Handler;
            auto Matcher = typeLoc(alignsWithExpansion(&Ctx, (Exp)))
                               .bind("root");
            Finder.addMatcher(Matcher, &Handler);
            Finder.matchAST(Ctx);
            for (auto &&M : Handler.Matches)
                Exp->ASTRoots.push_back(M);
        }

        // If the expansion only aligns with one node, then set this
        // as its aligned root
        Exp->AlignedRoot = (Exp->ASTRoots.size() == 1)
                               ? (&(Exp->ASTRoots.front()))
                               : nullptr;

        //// Find AST nodes aligned with each of the expansion's arguments

        for (auto &&Arg : Exp->Arguments)
        {
            // Match stmts
            {
                MatchFinder Finder;
                ExpansionMatchHandler Handler;
                auto Matcher = stmt(unless(anyOf(implicitCastExpr(),
                                                 implicitValueInitExpr())),
                                    isSpelledFromTokens(&Ctx, Arg.Tokens))
                                   .bind("root");
                Finder.addMatcher(Matcher, &Handler);
                Finder.matchAST(Ctx);
                for (auto &&M : Handler.Matches)
                    Arg.AlignedRoots.push_back(M);
            }

            // Match decls
            {
                MatchFinder Finder;
                ExpansionMatchHandler Handler;
                auto Matcher = decl(isSpelledFromTokens(&Ctx, Arg.Tokens))
                                   .bind("root");
                Finder.addMatcher(Matcher, &Handler);
                Finder.matchAST(Ctx);
                for (auto &&M : Handler.Matches)
                    Arg.AlignedRoots.push_back(M);
            }

            // Match type locs
            {
                MatchFinder Finder;
                ExpansionMatchHandler Handler;
                auto Matcher =
                    typeLoc(isSpelledFromTokens(&Ctx, Arg.Tokens))
                        .bind("root");
                Finder.addMatcher(Matcher, &Handler);
                Finder.matchAST(Ctx);
                for (auto &&M : Handler.Matches)
                    Arg.AlignedRoots.push_back(M);
            }
        }
    }
} // namespace cpp2c
