#define A_AND_B(A, B) ((A) && (B))

int main(int argc, char const *argv[])
{
    A_AND_B(1, 2);
    return 0;
}


// Expected invocation properties:
// Invocation	{     "Name" : "A_AND_B",     "DefinitionLocation" : "/maki/tests/logical_and.c:1:9",     "InvocationLocation" : "/maki/tests/logical_and.c:5:5",     "ASTKind" : "Expr",     "TypeSignature" : "int(int, int)",     "InvocationDepth" : 0,     "NumASTRoots" : 1,     "NumArguments" : 2,     "HasStringification" : false,     "HasTokenPasting" : false,     "HasAlignedArguments" : true,     "HasSameNameAsOtherDeclaration" : false,     "IsExpansionControlFlowStmt" : false,     "DoesBodyReferenceMacroDefinedAfterMacro" : false,     "DoesBodyReferenceDeclDeclaredAfterMacro" : false,     "DoesBodyContainDeclRefExpr" : false,     "DoesSubexpressionExpandedFromBodyHaveLocalType" : false,     "DoesSubexpressionExpandedFromBodyHaveTypeDefinedAfterMacro" : false,     "DoesAnyArgumentHaveSideEffects" : false,     "DoesAnyArgumentContainDeclRefExpr" : false,     "IsHygienic" : true,     "IsDefinitionLocationValid" : true,     "IsInvocationLocationValid" : true,     "IsObjectLike" : false,     "IsInvokedInMacroArgument" : false,     "IsNamePresentInCPPConditional" : false,     "IsExpansionICE" : true,     "IsExpansionTypeNull" : false,     "IsExpansionTypeAnonymous" : false,     "IsExpansionTypeLocalType" : false,     "IsExpansionTypeDefinedAfterMacro" : false,     "IsExpansionTypeVoid" : false,     "IsAnyArgumentTypeNull" : false,     "IsAnyArgumentTypeAnonymous" : false,     "IsAnyArgumentTypeLocalType" : false,     "IsAnyArgumentTypeDefinedAfterMacro" : false,     "IsAnyArgumentTypeVoid" : false,     "IsInvokedWhereModifiableValueRequired" : false,     "IsInvokedWhereAddressableValueRequired" : false,     "IsInvokedWhereICERequired" : false,     "IsAnyArgumentExpandedWhereModifiableValueRequired" : false,     "IsAnyArgumentExpandedWhereAddressableValueRequired" : false,     "IsAnyArgumentConditionallyEvaluated" : true,     "IsAnyArgumentNeverExpanded" : false,     "IsAnyArgumentNotAnExpression" : false  }
