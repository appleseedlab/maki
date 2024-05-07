#include "ASTUtils.hh"

namespace maki {
bool isInTree(const clang::Stmt *ST,
              std::function<bool(const clang::Stmt *)> pred) {
    if (!ST)
        return false;

    if (pred(ST))
        return true;

    for (auto &&child : ST->children())
        if (isInTree(child, pred))
            return true;

    return false;
}
} // namespace maki
