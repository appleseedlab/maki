#include "DeclStmtTypeLoc.hh"

#include "assert.h"

namespace cpp2c
{
    DeclStmtTypeLoc::DeclStmtTypeLoc(const clang::Decl *D) : D(D) {}
    DeclStmtTypeLoc::DeclStmtTypeLoc(const clang::Stmt *ST) : ST(ST) {}
    DeclStmtTypeLoc::DeclStmtTypeLoc(const clang::TypeLoc *TL) : TL(TL) {}

    void DeclStmtTypeLoc::dump()
    {
        assert(!(D && ST) && !(D && TL) && !(ST && TL) && "More than one type");
        if (D)
            D->dump();
        else if (ST)
            ST->dump();
        else if (TL)
            TL->getType().dump();
        else
            assert(!"No node to dump");
    }
} // namespace cpp2c
