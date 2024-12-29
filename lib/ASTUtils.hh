#pragma once

#include <clang/AST/ASTContext.h>
#include <clang/AST/Stmt.h>
#include <clang/AST/Type.h>
#include <clang/Basic/SourceLocation.h>
#include <functional>
#include <set>

namespace maki {

// Removes all sugar and qualifiers from the given type, getting the base type
// under pointers, typedefs, arrays, and typeof types.
const clang::Type *getBaseUnqualifiedTypeOrNull(const clang::Type *T);

// If T's base type is a tag type, returns the decl for that type. Otherwise
// returns nullptr.
clang::TagDecl *getBaseTypeTagDeclOrNull(const clang::Type *T);

// Returns whether T's base type is an anonymous (i.e., unnamed) type.
bool isBaseTypeAnonymous(const clang::Type *T);

// Returns whether T's base type is a locally-defined type.
bool isBaseTypeLocal(const clang::Type *T);

// Returns whether T's base type was defined after L.
bool isBaseTypeDefinedAfter(const clang::Type *T, clang::ASTContext &Ctx,
                            clang::SourceLocation L);

// Desugars T one step at a time, and returns whether any desugared type in the
// process is defined after L.
bool isAnyDesugaredTypeDefinedAfter(const clang::Type *T,
                                    clang::ASTContext &Ctx,
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

// Collect all subtrees of the given stmt using BFS.
std::set<const clang::Stmt *> subtrees(const clang::Stmt *ST);

// Returns true if the given statement is a compound statement, or a
// for/while/if statement whose body ends with a compound statement.
bool endsWithCompound(const clang::Stmt *ST);
} // namespace maki
