#pragma once

#include <clang/AST/DeclBase.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <vector>

namespace maki {
class DeclCollectorMatchHandler
    : public clang::ast_matchers::MatchFinder::MatchCallback {
public:
    std::vector<const clang::Decl *> Decls;

    virtual void
    run(const clang::ast_matchers::MatchFinder::MatchResult &Result) override;
};
} // namespace maki