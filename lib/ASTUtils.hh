#pragma once

#include <clang/AST/ASTContext.h>
#include <clang/AST/Stmt.h>
#include <clang/AST/Type.h>
#include <clang/Basic/SourceLocation.h>
#include <functional>
#include <set>

namespace maki {

// If this type is a tag type, returns the type's declaration. If the type is a
// typedef or elaborated type, then this function makes recursive call to find
// the underlying type.
clang::Decl *getTypeDeclOrNull(const clang::Type *T);

// Returns true if any type in T is an anonymous (i.e., unnamed) type.
bool hasAnonymousType(const clang::Type *T, clang::ASTContext &Ctx);

// Returns true if any type in T is a locally-defined type.
bool hasLocalType(const clang::Type *T, clang::ASTContext &Ctx);

// Returns true if any type in T was defined after L.
bool hasTypeDefinedAfter(const clang::Type *T, clang::ASTContext &Ctx,
                         clang::SourceLocation L);

// Returns true if a Stmt that satisfies the given predicate is a subtree of the
// given Stmt.
bool isInTree(const clang::Stmt *Stmt,
              std::function<bool(const clang::Stmt *)> pred);

// Returns true if ST is a descendant of a Node which can only have
// subexpressions that are constant expressions.
bool isDescendantOfNodeRequiringConstantExpression(clang::ASTContext &Ctx,
                                                   const clang::Stmt *ST);

// Returns true if ST is a descendant of a Node which can only have
// subexpressions that are integral constants expressions (e.g. in global
// variable initializers).
bool isDescendantOfNodeRequiringICE(clang::ASTContext &Ctx,
                                    const clang::Stmt *ST);

// Returns true if LHS is a subtree of RHS via BFS.
bool inTree(const clang::Stmt *LHS, const clang::Stmt *RHS);

// Returns true if the given predicate returns true for any type contained in
// the given type.
bool isInType(const clang::Type *T, clang::ASTContext &Ctx,
              std::function<bool(const clang::Type *)> pred);

// Collect all subtrees of the given stmt using BFS.
std::set<const clang::Stmt *> subtrees(const clang::Stmt *ST);

// Returns true if the given statement is a compound statement, or a
// for/while/if statement whose body ends with a compound statement.
bool endsWithCompound(const clang::Stmt *ST);
} // namespace maki
