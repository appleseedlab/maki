#pragma once

#include <clang/AST/Stmt.h>
#include <functional>

namespace maki {

// Returns true if a Stmt that satisfies the given predicate is a subtree of the
// given Stmt.
bool isInTree(const clang::Stmt *Stmt,
              std::function<bool(const clang::Stmt *)> pred);
} // namespace maki
