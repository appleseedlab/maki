#pragma once

#include "MacroExpansionNode.hh"
#include "StmtCollectorMatchHandler.hh"

#include "clang/AST/ASTContext.h"

namespace cpp2c
{

    // Checks that a given expansion is hygienic.
    // By hygienic, we mean that it satisfies Clinger and Rees'
    // strong hygiene condition for macros:
    // Local variables in the expansion must have been passed as arguments,
    // and the expansion must not create new declarations that can be
    // accessed outside of the expansion.
    // TODO: Check for new declarations!
    //       Right now we only check for unbound local vars.
    bool isHygienic(MacroExpansionNode *Expansion);

    bool isParameterSideEffectFree(MacroExpansionNode *Expansion);

    bool isLValueIndependent(MacroExpansionNode *Expansion);
} // namespace cpp2c
