#include "DeclCollectorMatchHandler.hh"

namespace cpp2c
{
    void DeclCollectorMatchHandler::run(
        const clang::ast_matchers::MatchFinder::MatchResult &Result)
    {
        if (auto ST = Result.Nodes.getNodeAs<clang::Decl>("root"))
            Decls.push_back(ST);
        else
            assert(!"Matched a non-Decl node");
    }
} // namespace cpp2c
