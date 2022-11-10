#include "DeclStmtTypeLoc.hh"

#include "assert.h"

namespace cpp2c
{
    DeclStmtTypeLoc::DeclStmtTypeLoc(const clang::Decl *D) : D(D) {}
    DeclStmtTypeLoc::DeclStmtTypeLoc(const clang::Stmt *ST) : ST(ST) {}
    DeclStmtTypeLoc::DeclStmtTypeLoc(const clang::TypeLoc *TL) : TL(TL) {}

    inline void DeclStmtTypeLoc::assertOneNonNull()
    {
        assert(!(D && ST) && !(D && TL) && !(ST && TL) && "More than one type");
    }

    void DeclStmtTypeLoc::dump()
    {
        assertOneNonNull();
        if (D)
            D->dump();
        else if (ST)
            ST->dump();
        else if (TL)
        {
            auto QT = TL->getType();
            if (!QT.isNull())
                QT.dump();
            else
                llvm::errs() << "<Null type>\n";
        }
        else
            assert(!"No node to dump");
    }

    clang::SourceRange DeclStmtTypeLoc::getSourceRange()
    {
        assertOneNonNull();
        if (D)
            return D->getSourceRange();
        else if (ST)
            return ST->getSourceRange();
        else if (TL)
            return TL->getSourceRange();
        else
            assert(!"No node to dump");
    }
} // namespace cpp2c
