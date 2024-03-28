#define SKIP_SPACES(p, limit) \
    do                        \
    {                         \
        char *lim = (limit);  \
        while (p < lim)       \
        {                     \
            if (*p++ != ' ')  \
            {                 \
                p--;          \
                break;        \
            }                 \
        }                     \
    } while (0)

int main(int argc, char const *argv[])
{
    char *goal = "FSE 23'";
    SKIP_SPACES(goal, (goal + 4));
    return 0;
}


// Expected invocation properties:
// Invocation	{     "Name" : "SKIP_SPACES",     "DefinitionLocation" : "/maki/tests/do_while.c:1:9",     "InvocationLocation" : "/maki/tests/do_while.c:18:5",     "ASTKind" : "Stmt",     "TypeSignature" : "void(char *, char *)",     "InvocationDepth" : 0,     "NumASTRoots" : 1,     "NumArguments" : 2,     "HasStringification" : false,     "HasTokenPasting" : false,     "HasAlignedArguments" : true,     "HasSameNameAsOtherDeclaration" : false,     "IsExpansionControlFlowStmt" : true,     "DoesBodyReferenceMacroDefinedAfterMacro" : false,     "DoesBodyReferenceDeclDeclaredAfterMacro" : true,     "DoesBodyContainDeclRefExpr" : true,     "DoesSubexpressionExpandedFromBodyHaveLocalType" : false,     "DoesSubexpressionExpandedFromBodyHaveTypeDefinedAfterMacro" : false,     "DoesAnyArgumentHaveSideEffects" : false,     "DoesAnyArgumentContainDeclRefExpr" : true,     "IsHygienic" : true,     "IsDefinitionLocationValid" : true,     "IsInvocationLocationValid" : true,     "IsObjectLike" : false,     "IsInvokedInMacroArgument" : false,     "IsNamePresentInCPPConditional" : false,     "IsExpansionICE" : false,     "IsExpansionTypeNull" : false,     "IsExpansionTypeAnonymous" : false,     "IsExpansionTypeLocalType" : false,     "IsExpansionTypeDefinedAfterMacro" : false,     "IsExpansionTypeVoid" : false,     "IsAnyArgumentTypeNull" : false,     "IsAnyArgumentTypeAnonymous" : false,     "IsAnyArgumentTypeLocalType" : false,     "IsAnyArgumentTypeDefinedAfterMacro" : false,     "IsAnyArgumentTypeVoid" : false,     "IsInvokedWhereModifiableValueRequired" : false,     "IsInvokedWhereAddressableValueRequired" : false,     "IsInvokedWhereICERequired" : false,     "IsAnyArgumentExpandedWhereModifiableValueRequired" : true,     "IsAnyArgumentExpandedWhereAddressableValueRequired" : false,     "IsAnyArgumentConditionallyEvaluated" : false,     "IsAnyArgumentNeverExpanded" : false,     "IsAnyArgumentNotAnExpression" : false  }
