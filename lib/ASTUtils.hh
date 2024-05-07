#pragma once

#include "clang/AST/Stmt.h"

#include <functional>

namespace maki {
bool isInTree(const clang::Stmt *ST,
              std::function<bool(const clang::Stmt *)> pred);
} // namespace maki
