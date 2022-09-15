#pragma once

#include "clang/AST/Decl.h"
#include "clang/AST/Stmt.h"
#include "clang/AST/TypeLoc.h"

namespace cpp2c
{
    class DeclStmtTypeLoc
    {
    private:
        inline void assertOneNonNull();

    public:
        const clang::Decl *D = nullptr;
        const clang::Stmt *ST = nullptr;
        const clang::TypeLoc *TL = nullptr;

        DeclStmtTypeLoc(const clang::Decl *D);
        DeclStmtTypeLoc(const clang::Stmt *ST);
        DeclStmtTypeLoc(const clang::TypeLoc *TL);

        void dump();

        clang::SourceRange getSourceRange();
    };
} // namespace cpp2c
