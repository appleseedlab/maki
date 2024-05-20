#include "ASTUtils.hh"
#include <clang/AST/Stmt.h>
#include <functional>

namespace maki {
bool isInTree(const clang::Stmt *Stmt,
              std::function<bool(const clang::Stmt *)> pred) {
    if (!Stmt) {
        return false;
    }

    if (pred(Stmt)) {
        return true;
    }

    for (auto &&child : Stmt->children()) {
        return isInTree(child, pred);
    }

    return false;
}
} // namespace maki
