#include "Properties.hh"
#include "ASTUtils.hh"
#include "StmtCollectorMatchHandler.hh"

#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

#include <stack>

namespace cpp2c
{

    // Returns true if the given expansion is hygienic.
    // An expansion is hygienic if it does not capture variables from its
    // caller's environment (unless they were passed as an argument),
    // and does not introduce any new variables into its caller's environment.
    // We only really have to check statements for variable capture, since
    // type locs and declarations always introduce new symbols.
    bool isHygienic(MacroExpansionNode *Expansion)
    {
        assert(Expansion->AlignedRoot && "Expansion must have an aligned root");

        if (Expansion->AlignedRoot->TL || Expansion->AlignedRoot->D)
            return false;

        // 1. Collect all local vars in the expansion
        std::vector<const clang::DeclRefExpr *> LocalVars;
        std::stack<const clang::Stmt *> Stk;
        Stk.push(Expansion->AlignedRoot->ST);
        while (!Stk.empty())
        {
            auto St = Stk.top();
            Stk.pop();

            if (!St)
                continue;

            for (auto &&Child : St->children())
                Stk.push(Child);

            auto DRE = clang::dyn_cast<clang::DeclRefExpr>(St);
            if (!DRE)
                continue;

            auto VD = clang::dyn_cast_or_null<clang::VarDecl>(DRE->getDecl());
            if (!VD)
                continue;

            if (!VD->hasLocalStorage())
                continue;

            LocalVars.push_back(DRE);
        };

        // 2. Check that all local vars, if any, were passed as arguments.
        //    If a local var was not passed as an argument, then it is
        //    captured from the caller's environment.
        for (auto &&DRE : LocalVars)
            for (auto &&Arg : Expansion->Arguments)
            {
                bool FromArg = false;
                for (auto &&AR : Arg.AlignedRoots)
                    FromArg |= isInTree(AR.ST, [&DRE](const clang::Stmt *St)
                                        { return DRE == St; });
                if (!FromArg)
                    return false;
            }
        return true;
    }

    // Returns true if none of the arguments passed to the expansion have
    // contain a subexpression with side-effects
    bool isParameterSideEffectFree(MacroExpansionNode *Expansion)
    {
        assert(Expansion->AlignedRoot && "Expansion must have aligned root");
        if (Expansion->AlignedRoot->TL || Expansion->AlignedRoot->D)
            return true;

        for (auto &&Arg : Expansion->Arguments)
            for (auto &&AR : Arg.AlignedRoots)
                if (isInTree(
                        AR.ST,
                        [](const clang::Stmt *St)
                        {
                            auto B = clang::dyn_cast<clang::BinaryOperator>(St);
                            auto U = clang::dyn_cast<clang::UnaryOperator>(St);
                            auto C = clang::dyn_cast<clang::CallExpr>(St);

                            return ((B && B->isAssignmentOp()) ||
                                    (U && (U->isIncrementDecrementOp())) ||
                                    C);
                        }))
                    return false;
        return true;
    }

    // Returns true if the given expansion is L-value independent.
    // An expansion is L-value independent if, for all of its subexpressions
    // which read the L-value of a declaration, that L-value would
    // remain the same under both dynamic and static scoping.
    bool isLValueIndependent(MacroExpansionNode *Expansion)
    {
        assert(Expansion->AlignedRoot &&
               Expansion->AlignedRoot->ST &&
               "Expansion must have an aligned Stmt root");

        // 1. Collect & exprs and side-effecting exprs
        std::vector<const clang::Expr *> LValueExprs;
        std::stack<const clang::Stmt *> Stk;
        Stk.push(Expansion->AlignedRoot->ST);

        auto OpCode_AO = clang::UnaryOperator::Opcode::UO_AddrOf;
        while (!Stk.empty())
        {
            auto St = Stk.top();
            Stk.pop();

            if (!St)
                continue;

            for (auto &&Child : St->children())
                Stk.push(Child);

            auto BO = clang::dyn_cast<clang::BinaryOperator>(St);
            auto UO = clang::dyn_cast<clang::UnaryOperator>(St);

            if (BO && BO->isAssignmentOp())
                LValueExprs.push_back(BO);
            else if (UO && (UO->getOpcode() == OpCode_AO ||
                            UO->isIncrementDecrementOp()))
                LValueExprs.push_back(UO);
        }

        // 2. Check that every expr either comes from an argument, or
        //    does not read the L-value of a declaration whose L-value
        //    would change under lexical scoping
        for (auto &&LVE : LValueExprs)
        {
            // Check if the entire expr came from an argument
            bool cameFromArg = false;
            for (auto &&Arg : Expansion->Arguments)
                for (auto &&AR : Arg.AlignedRoots)
                    cameFromArg |= isInTree(AR.ST,
                                            [&LVE](const clang::Stmt *St)
                                            { return St == LVE; });
            if (cameFromArg)
                continue;

            // If not, the expression is L-value independent if
            // none of the operand's subtrees came from an argument
            const clang::Expr *Operand = nullptr;
            if (auto UO = clang::dyn_cast<clang::UnaryOperator>(LVE))
                Operand = UO->getSubExpr();
            else if (auto BO = clang::dyn_cast<clang::BinaryOperator>(LVE))
                Operand = BO->getLHS();
            else
                assert(!"Matched an operator that was not"
                        "an address of operator, or unary/binary"
                        "assignment operator");

            // Check that none of the operand's subtrees came from an argument
            for (auto &&Arg : Expansion->Arguments)
                for (auto &&AR : Arg.AlignedRoots)
                    if (isInTree(
                            Operand,
                            [&AR](const clang::Stmt *OperandSubTr)
                            { return isInTree(
                                  AR.ST,
                                  [&OperandSubTr](const clang::Stmt *ARSubTr)
                                  { return OperandSubTr == ARSubTr; }); }))
                        return false;
        }
        return true;
    }
} // namespace cpp2c
