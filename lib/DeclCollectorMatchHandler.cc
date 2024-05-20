#include "DeclCollectorMatchHandler.hh"
#include <cassert>
#include <clang/AST/DeclBase.h>

namespace maki {
void DeclCollectorMatchHandler::run(
    const clang::ast_matchers::MatchFinder::MatchResult &Result) {
    if (auto Decl = Result.Nodes.getNodeAs<clang::Decl>("root")) {
        Decls.push_back(Decl);
    } else {
        assert(!"Matched a non-Decl node");
    }
}
} // namespace maki
