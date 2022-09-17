#include "ExpansionArgumentMatchHandler.hh"

#include "clang/AST/Expr.h"

#include <assert.h>

namespace cpp2c
{

    ExpansionArgumentMatchHandler::ExpansionArgumentMatchHandler(MacroExpansionArgument *Argument)
        : Argument(Argument) {}

    void ExpansionArgumentMatchHandler::run(
        const clang::ast_matchers::MatchFinder::MatchResult &Result)
    {
        if (const auto D = Result.Nodes.getNodeAs<clang::Decl>("root"))
            Argument->ASTRoots.push_back(DeclStmtTypeLoc(D));
        else if (const auto ST = Result.Nodes.getNodeAs<clang::Stmt>("root"))
            Argument->ASTRoots.push_back(DeclStmtTypeLoc(ST));
        else if (const auto TL = Result.Nodes.getNodeAs<clang::TypeLoc>("root"))
            Argument->ASTRoots.push_back(DeclStmtTypeLoc(TL));
        else
            assert(!"Matched a node that was node a Decl/Stmt/TypeLoc");
    }
} // namespace cpp2c
