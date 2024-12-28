#include "ASTUtils.hh"
#include <clang/AST/ASTContext.h>
#include <clang/AST/ASTTypeTraits.h>
#include <clang/AST/Decl.h>
#include <clang/AST/ParentMapContext.h>
#include <clang/AST/Stmt.h>
#include <clang/AST/Type.h>
#include <clang/Basic/LLVM.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Basic/SourceManager.h>
#include <functional>
#include <llvm-17/llvm/Support/raw_ostream.h>
#include <queue>
#include <set>

namespace maki {

const clang::Type *getBaseUnqualifiedTypeOrNull(const clang::Type *T) {
    while (T) {
        if (T->isArrayType()) {
            T = T->getBaseElementTypeUnsafe();
        } else if (T->isAnyPointerType()) {
            T = T->getPointeeType().getTypePtrOrNull();
        } else if (auto ET = clang::dyn_cast<clang::ElaboratedType>(T)) {
            T = ET->desugar().getTypePtrOrNull();
        } else if (auto TD = clang::dyn_cast<clang::TypedefType>(T)) {
            T = TD->desugar().getTypePtrOrNull();
        } else if (auto ToT = clang::dyn_cast<clang::TypeOfType>(T)) {
            T = ToT->getUnmodifiedType().getTypePtrOrNull();
        } else {
            return T->getUnqualifiedDesugaredType();
        }
    }
    return T ? T->getUnqualifiedDesugaredType() : nullptr;
}

clang::TagDecl *getBaseTypeTagDeclOrNull(const clang::Type *T) {
    T = getBaseUnqualifiedTypeOrNull(T);
    if (auto TT = clang::dyn_cast_or_null<clang::TagType>(T)) {
        return TT->getAsTagDecl();
    }
    return nullptr;
}

bool isBaseTypeAnonymous(const clang::Type *T) {
    auto TD = getBaseTypeTagDeclOrNull(T);
    auto ND = clang::dyn_cast_or_null<clang::NamedDecl>(TD);

    return ND && (!ND->getIdentifier() || ND->getName().empty());
}

bool isBaseTypeLocal(const clang::Type *T) {
    auto TD = getBaseTypeTagDeclOrNull(T);
    auto DCtx = TD ? TD->getDeclContext() : nullptr;

    return DCtx && !DCtx->isTranslationUnit();
}

bool isBaseTypeDefinedAfter(const clang::Type *T, clang::ASTContext &Ctx,
                            clang::SourceLocation L) {
    auto &SM = Ctx.getSourceManager();
    auto TD = getBaseTypeTagDeclOrNull(T);
    auto DLoc = TD ? TD->getLocation() : clang::SourceLocation();
    auto DFLoc = DLoc.isValid() ? SM.getFileLoc(DLoc) : clang::SourceLocation();

    return DFLoc.isValid() && SM.isBeforeInTranslationUnit(L, DFLoc);
}

bool isAnyDesugaredTypeDefinedAfter(const clang::Type *T,
                                    clang::ASTContext &Ctx,
                                    clang::SourceLocation L) {
    auto &SM = Ctx.getSourceManager();

    while (T) {
        if (T->isArrayType()) {
            T = T->getBaseElementTypeUnsafe();
        } else if (T->isAnyPointerType()) {
            T = T->getPointeeType().getTypePtrOrNull();
        } else if (auto ET = clang::dyn_cast<clang::ElaboratedType>(T)) {
            T = ET->desugar().getTypePtrOrNull();
        } else if (auto TT = clang::dyn_cast_or_null<clang::TagType>(T)) {
            auto TagDecl = TT->getDecl();
            auto DLoc = TagDecl ? TagDecl->getLocation() :
                                  clang::SourceLocation();
            auto DFLoc = DLoc.isValid() ? SM.getFileLoc(DLoc) :
                                          clang::SourceLocation();

            return DFLoc.isValid() && SM.isBeforeInTranslationUnit(L, DFLoc);
        } else if (auto TD = clang::dyn_cast<clang::TypedefType>(T)) {
            auto TND = TD->getDecl();
            auto DLoc = TND ? TND->getLocation() : clang::SourceLocation();
            auto DFLoc = DLoc.isValid() ? SM.getFileLoc(DLoc) :
                                          clang::SourceLocation();
            if (DFLoc.isValid() && SM.isBeforeInTranslationUnit(L, DFLoc)) {
                return true;
            }

            T = TD->desugar().getTypePtrOrNull();
        } else if (auto ToT = clang::dyn_cast<clang::TypeOfType>(T)) {
            T = ToT->getUnmodifiedType().getTypePtrOrNull();
        } else {
            break;
        }
    }

    return false;
}

bool isInTree(const clang::Stmt *Stmt,
              std::function<bool(const clang::Stmt *)> pred) {
    if (!Stmt) {
        return false;
    }

    if (pred(Stmt)) {
        return true;
    }

    for (auto &&child : Stmt->children()) {
        if (isInTree(child, pred)) {
            return true;
        }
    }

    return false;
}

bool isDescendantOfNodeRequiringConstantExpression(clang::ASTContext &Ctx,
                                                   const clang::Stmt *ST) {
    if (!ST) {
        return false;
    }

    std::queue<clang::DynTypedNode> Q;
    for (auto P : Ctx.getParents(*ST)) {
        Q.push(P);
    }

    while (!Q.empty()) {
        auto Cur = Q.front();
        Q.pop();
        if (auto VD = Cur.get<clang::VarDecl>()) {
            // Check if the expression is the initializer of a variable that has
            // global or static storage.
            if (VD->hasGlobalStorage() || VD->isStaticLocal()) {
                return true;
            }
        }
        for (auto P : Ctx.getParents(Cur)) {
            Q.push(P);
        }
    }
    return false;
}

bool isDescendantOfNodeRequiringICE(clang::ASTContext &Ctx,
                                    const clang::Stmt *ST) {
    if (!ST) {
        return false;
    }

    std::queue<clang::DynTypedNode> Q;
    for (auto P : Ctx.getParents(*ST)) {
        Q.push(P);
    }
    while (!Q.empty()) {
        auto Cur = Q.front();
        Q.pop();

        if (Cur.get<clang::CaseStmt>() || Cur.get<clang::EnumDecl>()) {
            return true;
        }

        if (auto FD = Cur.get<clang::FieldDecl>()) {
            if (FD->isBitField()) {
                return true;
            }
            if (auto Type = FD->getType(); clang::isa<clang::ArrayType>(Type)) {
                return true;
            }
        }

        if (auto VD = Cur.get<clang::ValueDecl>()) {
            // Check if this expression is the size specifier for an array type
            // for a value declaration.
            if (auto Type = VD->getType(); Type->isConstantArrayType()) {
                return true;
            } else if (auto VarD = clang::dyn_cast<clang::VarDecl>(VD)) {
                // Check if the expression is the initializer of an integer
                // variable that has global or static storage.
                if (VarD->hasGlobalStorage() || VarD->isStaticLocal()) {
                    if (VarD->getType()->isIntegerType()) {
                        return true;
                    }
                }
            }
        }

        // Check if this expression is the size specifier for an array type
        // inside a typedef.
        if (auto TD = Cur.get<clang::TypedefDecl>()) {
            if (TD->getUnderlyingType()->isConstantArrayType()) {
                return true;
            }
        }

        for (auto P : Ctx.getParents(Cur)) {
            Q.push(P);
        }
    }
    return false;
}

bool inTree(const clang::Stmt *LHS, const clang::Stmt *RHS) {
    std::queue<const clang::Stmt *> Q({ RHS });
    while (!Q.empty()) {
        auto Cur = Q.front();
        Q.pop();
        if (LHS == Cur) {
            return true;
        }
        if (Cur) {
            for (auto &&Child : Cur->children()) {
                Q.push(Child);
            }
        }
    }
    return false;
}

std::set<const clang::Stmt *> subtrees(const clang::Stmt *ST) {
    std::set<const clang::Stmt *> Subtrees;
    if (!ST) {
        return Subtrees;
    }

    std::queue<const clang::Stmt *> Q({ ST });
    while (!Q.empty()) {
        auto Cur = Q.front();
        Q.pop();
        if (Cur) {
            Subtrees.insert(Cur);
            for (auto &&Child : Cur->children()) {
                Q.push(Child);
            }
        }
    }
    return Subtrees;
}

bool endsWithCompound(const clang::Stmt *ST) {
    if (!ST) {
        return false;
    } else if (clang::isa<clang::CompoundStmt>(ST)) {
        return true;
    } else if (auto While = clang::dyn_cast<clang::WhileStmt>(ST)) {
        return endsWithCompound(While->getBody());
    } else if (auto For = clang::dyn_cast<clang::ForStmt>(ST)) {
        return endsWithCompound(For->getBody());
    } else if (auto If = clang::dyn_cast<clang::IfStmt>(ST)) {
        if (auto Else = If->getElse()) {
            return endsWithCompound(Else);
        } else {
            return endsWithCompound(If->getThen());
        }
    } else if (auto Switch = clang::dyn_cast<clang::SwitchStmt>(ST)) {
        return endsWithCompound(Switch->getBody());
    } else {
        return false;
    }
}
} // namespace maki
