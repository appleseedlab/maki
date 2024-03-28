// The bodies of all invocations are not modified values

#define INC_G g++

int g = 0;

int main(int argc, char const *argv[])
{
    // Impure, but not expanded where a modifiable value is required
    INC_G;

    {
        int g = 0;
        // Impure, but not expanded where a modifiable value is required
        INC_G;
    }

    return 0;
}


// Expected invocation properties:
// Invocation	{     "Name" : "INC_G",     "DefinitionLocation" : "/maki/tests/unmodified_invocation.c:3:9",     "InvocationLocation" : "/maki/tests/unmodified_invocation.c:10:5",     "ASTKind" : "Expr",     "TypeSignature" : "int",     "InvocationDepth" : 0,     "NumASTRoots" : 1,     "NumArguments" : 0,     "HasStringification" : false,     "HasTokenPasting" : false,     "HasAlignedArguments" : true,     "HasSameNameAsOtherDeclaration" : false,     "IsExpansionControlFlowStmt" : false,     "DoesBodyReferenceMacroDefinedAfterMacro" : false,     "DoesBodyReferenceDeclDeclaredAfterMacro" : true,     "DoesBodyContainDeclRefExpr" : true,     "DoesSubexpressionExpandedFromBodyHaveLocalType" : false,     "DoesSubexpressionExpandedFromBodyHaveTypeDefinedAfterMacro" : false,     "DoesAnyArgumentHaveSideEffects" : false,     "DoesAnyArgumentContainDeclRefExpr" : false,     "IsHygienic" : true,     "IsDefinitionLocationValid" : true,     "IsInvocationLocationValid" : true,     "IsObjectLike" : true,     "IsInvokedInMacroArgument" : false,     "IsNamePresentInCPPConditional" : false,     "IsExpansionICE" : false,     "IsExpansionTypeNull" : false,     "IsExpansionTypeAnonymous" : false,     "IsExpansionTypeLocalType" : false,     "IsExpansionTypeDefinedAfterMacro" : false,     "IsExpansionTypeVoid" : false,     "IsAnyArgumentTypeNull" : false,     "IsAnyArgumentTypeAnonymous" : true,     "IsAnyArgumentTypeLocalType" : false,     "IsAnyArgumentTypeDefinedAfterMacro" : false,     "IsAnyArgumentTypeVoid" : false,     "IsInvokedWhereModifiableValueRequired" : false,     "IsInvokedWhereAddressableValueRequired" : false,     "IsInvokedWhereICERequired" : false,     "IsAnyArgumentExpandedWhereModifiableValueRequired" : false,     "IsAnyArgumentExpandedWhereAddressableValueRequired" : false,     "IsAnyArgumentConditionallyEvaluated" : false,     "IsAnyArgumentNeverExpanded" : false,     "IsAnyArgumentNotAnExpression" : false  }
// Invocation	{     "Name" : "INC_G",     "DefinitionLocation" : "/maki/tests/unmodified_invocation.c:3:9",     "InvocationLocation" : "/maki/tests/unmodified_invocation.c:15:9",     "ASTKind" : "Expr",     "TypeSignature" : "int",     "InvocationDepth" : 0,     "NumASTRoots" : 1,     "NumArguments" : 0,     "HasStringification" : false,     "HasTokenPasting" : false,     "HasAlignedArguments" : true,     "HasSameNameAsOtherDeclaration" : false,     "IsExpansionControlFlowStmt" : false,     "DoesBodyReferenceMacroDefinedAfterMacro" : false,     "DoesBodyReferenceDeclDeclaredAfterMacro" : true,     "DoesBodyContainDeclRefExpr" : true,     "DoesSubexpressionExpandedFromBodyHaveLocalType" : false,     "DoesSubexpressionExpandedFromBodyHaveTypeDefinedAfterMacro" : false,     "DoesAnyArgumentHaveSideEffects" : false,     "DoesAnyArgumentContainDeclRefExpr" : false,     "IsHygienic" : false,     "IsDefinitionLocationValid" : true,     "IsInvocationLocationValid" : true,     "IsObjectLike" : true,     "IsInvokedInMacroArgument" : false,     "IsNamePresentInCPPConditional" : false,     "IsExpansionICE" : false,     "IsExpansionTypeNull" : false,     "IsExpansionTypeAnonymous" : false,     "IsExpansionTypeLocalType" : false,     "IsExpansionTypeDefinedAfterMacro" : false,     "IsExpansionTypeVoid" : false,     "IsAnyArgumentTypeNull" : false,     "IsAnyArgumentTypeAnonymous" : true,     "IsAnyArgumentTypeLocalType" : false,     "IsAnyArgumentTypeDefinedAfterMacro" : false,     "IsAnyArgumentTypeVoid" : false,     "IsInvokedWhereModifiableValueRequired" : false,     "IsInvokedWhereAddressableValueRequired" : false,     "IsInvokedWhereICERequired" : false,     "IsAnyArgumentExpandedWhereModifiableValueRequired" : false,     "IsAnyArgumentExpandedWhereAddressableValueRequired" : false,     "IsAnyArgumentConditionallyEvaluated" : false,     "IsAnyArgumentNeverExpanded" : false,     "IsAnyArgumentNotAnExpression" : false  }
