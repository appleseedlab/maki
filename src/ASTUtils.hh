#pragma once

#include "clang/AST/Stmt.h"

#include <functional>

namespace cpp2c {
bool isInTree(const clang::Stmt *ST,
              std::function<bool(const clang::Stmt *)> pred);
} // namespace cpp2c
