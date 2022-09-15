#pragma once

#include "clang/AST/Decl.h"
#include "clang/AST/Stmt.h"

namespace cpp2c
{
    class DeclStmt
    {
    public:
        const clang::Decl *D = nullptr;
        const clang::Stmt *ST = nullptr;

        DeclStmt(const clang::Decl *D);
        DeclStmt(const clang::Stmt *St);

        void dump();
    };
} // namespace cpp2c
