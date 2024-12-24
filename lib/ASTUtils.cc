#include "ASTUtils.hh"
#include "Logging.hh"
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
#include <queue>
#include <set>

namespace maki {
clang::Decl *getTypeDeclOrNull(const clang::Type *T) {
    if (!T) {
        return nullptr;
    }

    if (auto TD = clang::dyn_cast<clang::TypedefType>(T)) {
        return getTypeDeclOrNull(TD->desugar().getTypePtrOrNull());
    } else if (auto TD = clang::dyn_cast<clang::TagType>(T)) {
        return TD->getDecl();
    } else if (auto ET = clang::dyn_cast<clang::ElaboratedType>(T)) {
        return getTypeDeclOrNull(ET->desugar().getTypePtrOrNull());
    } else {
        return nullptr;
    }
}

bool hasAnonymousType(const clang::Type *T, clang::ASTContext &Ctx) {
    return isInType(T, Ctx, [](const clang::Type *T) {
        if (!T) {
            return false;
        }

        auto D = getTypeDeclOrNull(T);
        if (!D) {
            return false;
        }

        auto ND = clang::dyn_cast<clang::NamedDecl>(D);
        if (!ND) {
            return false;
        }

        return ND->getIdentifier() == nullptr || ND->getName().empty();
    });
}

bool hasLocalType(const clang::Type *T, clang::ASTContext &Ctx) {
    return isInType(T, Ctx, [](const clang::Type *T) {
        if (!T) {
            return false;
        }

        auto D = getTypeDeclOrNull(T);
        if (!D) {
            return false;
        }

        auto DCtx = D->getDeclContext();

        return !DCtx->isTranslationUnit();
    });
}

bool hasTypeDefinedAfter(const clang::Type *T, clang::ASTContext &Ctx,
                         clang::SourceLocation L, bool IgnoreTypedefs) {
    auto &SM = Ctx.getSourceManager();

    if (!T) {
        return false;
    }

    if (!IgnoreTypedefs) {
        auto TD = clang::dyn_cast<clang::TypedefType>(T);
        if (!TD) {
            auto ET = clang::dyn_cast<clang::ElaboratedType>(T);
            if (ET) {
                TD = clang::dyn_cast<clang::TypedefType>(ET->getNamedType());
            }
        }
        if (TD) {
            auto D = TD->getDecl();
            auto DLoc = D->getLocation();
            if (DLoc.isValid()) {
                auto DFLoc = SM.getFileLoc(DLoc);
                if (DFLoc.isValid()) {
                    return SM.isBeforeInTranslationUnit(L, DFLoc);
                }
            }
        }
    }

    return isInType(T, Ctx, [&SM, L](const clang::Type *T) {
        if (!T) {
            return false;
        }

        auto *D = getTypeDeclOrNull(T);

        if (!D) {
            return false;
        }

        auto DLoc = D->getLocation();
        if (DLoc.isInvalid()) {
            return false;
        }

        auto DFLoc = SM.getFileLoc(DLoc);
        if (DFLoc.isInvalid()) {
            return false;
        }

        return SM.isBeforeInTranslationUnit(L, DFLoc);
    });
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

bool isInType(const clang::Type *T, clang::ASTContext &Ctx,
              std::function<bool(const clang::Type *)> pred) {
    debug("Checking if T is a pointer type");
    while (T && (T->isAnyPointerType() || T->isArrayType())) {
        if (T->isAnyPointerType()) {
            T = T->getPointeeType().getTypePtrOrNull();
        } else if (T->isArrayType()) {
            T = T->getBaseElementTypeUnsafe();
        }
    }

    return pred(T);
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
