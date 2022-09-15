#include "DeclStmt.hh"

#include "assert.h"

namespace cpp2c
{
    DeclStmt::DeclStmt(const clang::Decl *D) : D(D) {}
    DeclStmt::DeclStmt(const clang::Stmt *ST) : ST(ST) {}

    void DeclStmt::dump()
    {
        assert(!(D && ST) && "Decl and Stmt");
        if (D)
            D->dump();
        else if (ST)
            ST->dump();
        else
            assert(!"No node to dump");
    }
} // namespace cpp2c
