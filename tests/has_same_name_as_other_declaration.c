#define A 1

#define FOO(A, B, C) 2

int bar = 0;

#define bar() 3

struct baz
{
};

#define baz() 4

int buzz()
{
    return 5;
}

#define buzz() 5

enum fizz
{
    fizz1,
};

#define fizz() 6

union fuzz
{
};

#define fuzz() 7

typedef int faz;

#define faz int

int main(int argc, char const *argv[])
{
    // A parameter has the same name as another declaration
    FOO(1, 2, 3);

    // Has the same name as another declaration (variable declaration)
    bar();

    // Has the same name as another declaration (struct declaration)
    baz();

    // Has the same name as another declaration (function declaration)
    buzz();

    // Has the same name as another declaration (enum declaration)
    fizz();

    // Has the same name as another declaration (union declaration)
    fuzz();

    // Has the same name as another declaration (typedef declaration)
    faz x;

    // TODO: Test the opposite case as well (doesn't have same name as other declaration)

    return 0;
}


// Expected invocation properties:
// Invocation	{     "Name" : "FOO",     "DefinitionLocation" : "/maki/tests/has_same_name_as_other_declaration.c:3:9",     "InvocationLocation" : "/maki/tests/has_same_name_as_other_declaration.c:42:5",     "ASTKind" : "Expr",     "TypeSignature" : "int(, , )",     "InvocationDepth" : 0,     "NumASTRoots" : 1,     "NumArguments" : 3,     "HasStringification" : false,     "HasTokenPasting" : false,     "HasAlignedArguments" : true,     "HasSameNameAsOtherDeclaration" : true,     "IsExpansionControlFlowStmt" : false,     "DoesBodyReferenceMacroDefinedAfterMacro" : false,     "DoesBodyReferenceDeclDeclaredAfterMacro" : false,     "DoesBodyContainDeclRefExpr" : false,     "DoesSubexpressionExpandedFromBodyHaveLocalType" : false,     "DoesSubexpressionExpandedFromBodyHaveTypeDefinedAfterMacro" : false,     "DoesAnyArgumentHaveSideEffects" : false,     "DoesAnyArgumentContainDeclRefExpr" : false,     "IsHygienic" : true,     "IsDefinitionLocationValid" : true,     "IsInvocationLocationValid" : true,     "IsObjectLike" : false,     "IsInvokedInMacroArgument" : false,     "IsNamePresentInCPPConditional" : false,     "IsExpansionICE" : true,     "IsExpansionTypeNull" : false,     "IsExpansionTypeAnonymous" : false,     "IsExpansionTypeLocalType" : false,     "IsExpansionTypeDefinedAfterMacro" : false,     "IsExpansionTypeVoid" : false,     "IsAnyArgumentTypeNull" : false,     "IsAnyArgumentTypeAnonymous" : true,     "IsAnyArgumentTypeLocalType" : false,     "IsAnyArgumentTypeDefinedAfterMacro" : false,     "IsAnyArgumentTypeVoid" : false,     "IsInvokedWhereModifiableValueRequired" : false,     "IsInvokedWhereAddressableValueRequired" : false,     "IsInvokedWhereICERequired" : false,     "IsAnyArgumentExpandedWhereModifiableValueRequired" : false,     "IsAnyArgumentExpandedWhereAddressableValueRequired" : false,     "IsAnyArgumentConditionallyEvaluated" : false,     "IsAnyArgumentNeverExpanded" : true,     "IsAnyArgumentNotAnExpression" : false  }
// Invocation	{     "Name" : "bar",     "DefinitionLocation" : "/maki/tests/has_same_name_as_other_declaration.c:7:9",     "InvocationLocation" : "/maki/tests/has_same_name_as_other_declaration.c:45:5",     "ASTKind" : "Expr",     "TypeSignature" : "int()",     "InvocationDepth" : 0,     "NumASTRoots" : 1,     "NumArguments" : 0,     "HasStringification" : false,     "HasTokenPasting" : false,     "HasAlignedArguments" : true,     "HasSameNameAsOtherDeclaration" : true,     "IsExpansionControlFlowStmt" : false,     "DoesBodyReferenceMacroDefinedAfterMacro" : false,     "DoesBodyReferenceDeclDeclaredAfterMacro" : false,     "DoesBodyContainDeclRefExpr" : false,     "DoesSubexpressionExpandedFromBodyHaveLocalType" : false,     "DoesSubexpressionExpandedFromBodyHaveTypeDefinedAfterMacro" : false,     "DoesAnyArgumentHaveSideEffects" : false,     "DoesAnyArgumentContainDeclRefExpr" : false,     "IsHygienic" : true,     "IsDefinitionLocationValid" : true,     "IsInvocationLocationValid" : true,     "IsObjectLike" : false,     "IsInvokedInMacroArgument" : false,     "IsNamePresentInCPPConditional" : false,     "IsExpansionICE" : true,     "IsExpansionTypeNull" : false,     "IsExpansionTypeAnonymous" : false,     "IsExpansionTypeLocalType" : false,     "IsExpansionTypeDefinedAfterMacro" : false,     "IsExpansionTypeVoid" : false,     "IsAnyArgumentTypeNull" : false,     "IsAnyArgumentTypeAnonymous" : true,     "IsAnyArgumentTypeLocalType" : false,     "IsAnyArgumentTypeDefinedAfterMacro" : false,     "IsAnyArgumentTypeVoid" : false,     "IsInvokedWhereModifiableValueRequired" : false,     "IsInvokedWhereAddressableValueRequired" : false,     "IsInvokedWhereICERequired" : false,     "IsAnyArgumentExpandedWhereModifiableValueRequired" : false,     "IsAnyArgumentExpandedWhereAddressableValueRequired" : false,     "IsAnyArgumentConditionallyEvaluated" : false,     "IsAnyArgumentNeverExpanded" : true,     "IsAnyArgumentNotAnExpression" : false  }
// Invocation	{     "Name" : "baz",     "DefinitionLocation" : "/maki/tests/has_same_name_as_other_declaration.c:13:9",     "InvocationLocation" : "/maki/tests/has_same_name_as_other_declaration.c:48:5",     "ASTKind" : "Expr",     "TypeSignature" : "int()",     "InvocationDepth" : 0,     "NumASTRoots" : 1,     "NumArguments" : 0,     "HasStringification" : false,     "HasTokenPasting" : false,     "HasAlignedArguments" : true,     "HasSameNameAsOtherDeclaration" : true,     "IsExpansionControlFlowStmt" : false,     "DoesBodyReferenceMacroDefinedAfterMacro" : false,     "DoesBodyReferenceDeclDeclaredAfterMacro" : false,     "DoesBodyContainDeclRefExpr" : false,     "DoesSubexpressionExpandedFromBodyHaveLocalType" : false,     "DoesSubexpressionExpandedFromBodyHaveTypeDefinedAfterMacro" : false,     "DoesAnyArgumentHaveSideEffects" : false,     "DoesAnyArgumentContainDeclRefExpr" : false,     "IsHygienic" : true,     "IsDefinitionLocationValid" : true,     "IsInvocationLocationValid" : true,     "IsObjectLike" : false,     "IsInvokedInMacroArgument" : false,     "IsNamePresentInCPPConditional" : false,     "IsExpansionICE" : true,     "IsExpansionTypeNull" : false,     "IsExpansionTypeAnonymous" : false,     "IsExpansionTypeLocalType" : false,     "IsExpansionTypeDefinedAfterMacro" : false,     "IsExpansionTypeVoid" : false,     "IsAnyArgumentTypeNull" : false,     "IsAnyArgumentTypeAnonymous" : true,     "IsAnyArgumentTypeLocalType" : false,     "IsAnyArgumentTypeDefinedAfterMacro" : false,     "IsAnyArgumentTypeVoid" : false,     "IsInvokedWhereModifiableValueRequired" : false,     "IsInvokedWhereAddressableValueRequired" : false,     "IsInvokedWhereICERequired" : false,     "IsAnyArgumentExpandedWhereModifiableValueRequired" : false,     "IsAnyArgumentExpandedWhereAddressableValueRequired" : false,     "IsAnyArgumentConditionallyEvaluated" : false,     "IsAnyArgumentNeverExpanded" : true,     "IsAnyArgumentNotAnExpression" : false  }
// Invocation	{     "Name" : "buzz",     "DefinitionLocation" : "/maki/tests/has_same_name_as_other_declaration.c:20:9",     "InvocationLocation" : "/maki/tests/has_same_name_as_other_declaration.c:51:5",     "ASTKind" : "Expr",     "TypeSignature" : "int()",     "InvocationDepth" : 0,     "NumASTRoots" : 1,     "NumArguments" : 0,     "HasStringification" : false,     "HasTokenPasting" : false,     "HasAlignedArguments" : true,     "HasSameNameAsOtherDeclaration" : true,     "IsExpansionControlFlowStmt" : false,     "DoesBodyReferenceMacroDefinedAfterMacro" : false,     "DoesBodyReferenceDeclDeclaredAfterMacro" : false,     "DoesBodyContainDeclRefExpr" : false,     "DoesSubexpressionExpandedFromBodyHaveLocalType" : false,     "DoesSubexpressionExpandedFromBodyHaveTypeDefinedAfterMacro" : false,     "DoesAnyArgumentHaveSideEffects" : false,     "DoesAnyArgumentContainDeclRefExpr" : false,     "IsHygienic" : true,     "IsDefinitionLocationValid" : true,     "IsInvocationLocationValid" : true,     "IsObjectLike" : false,     "IsInvokedInMacroArgument" : false,     "IsNamePresentInCPPConditional" : false,     "IsExpansionICE" : true,     "IsExpansionTypeNull" : false,     "IsExpansionTypeAnonymous" : false,     "IsExpansionTypeLocalType" : false,     "IsExpansionTypeDefinedAfterMacro" : false,     "IsExpansionTypeVoid" : false,     "IsAnyArgumentTypeNull" : false,     "IsAnyArgumentTypeAnonymous" : true,     "IsAnyArgumentTypeLocalType" : false,     "IsAnyArgumentTypeDefinedAfterMacro" : false,     "IsAnyArgumentTypeVoid" : false,     "IsInvokedWhereModifiableValueRequired" : false,     "IsInvokedWhereAddressableValueRequired" : false,     "IsInvokedWhereICERequired" : false,     "IsAnyArgumentExpandedWhereModifiableValueRequired" : false,     "IsAnyArgumentExpandedWhereAddressableValueRequired" : false,     "IsAnyArgumentConditionallyEvaluated" : false,     "IsAnyArgumentNeverExpanded" : true,     "IsAnyArgumentNotAnExpression" : false  }
// Invocation	{     "Name" : "fizz",     "DefinitionLocation" : "/maki/tests/has_same_name_as_other_declaration.c:27:9",     "InvocationLocation" : "/maki/tests/has_same_name_as_other_declaration.c:54:5",     "ASTKind" : "Expr",     "TypeSignature" : "int()",     "InvocationDepth" : 0,     "NumASTRoots" : 1,     "NumArguments" : 0,     "HasStringification" : false,     "HasTokenPasting" : false,     "HasAlignedArguments" : true,     "HasSameNameAsOtherDeclaration" : true,     "IsExpansionControlFlowStmt" : false,     "DoesBodyReferenceMacroDefinedAfterMacro" : false,     "DoesBodyReferenceDeclDeclaredAfterMacro" : false,     "DoesBodyContainDeclRefExpr" : false,     "DoesSubexpressionExpandedFromBodyHaveLocalType" : false,     "DoesSubexpressionExpandedFromBodyHaveTypeDefinedAfterMacro" : false,     "DoesAnyArgumentHaveSideEffects" : false,     "DoesAnyArgumentContainDeclRefExpr" : false,     "IsHygienic" : true,     "IsDefinitionLocationValid" : true,     "IsInvocationLocationValid" : true,     "IsObjectLike" : false,     "IsInvokedInMacroArgument" : false,     "IsNamePresentInCPPConditional" : false,     "IsExpansionICE" : true,     "IsExpansionTypeNull" : false,     "IsExpansionTypeAnonymous" : false,     "IsExpansionTypeLocalType" : false,     "IsExpansionTypeDefinedAfterMacro" : false,     "IsExpansionTypeVoid" : false,     "IsAnyArgumentTypeNull" : false,     "IsAnyArgumentTypeAnonymous" : true,     "IsAnyArgumentTypeLocalType" : false,     "IsAnyArgumentTypeDefinedAfterMacro" : false,     "IsAnyArgumentTypeVoid" : false,     "IsInvokedWhereModifiableValueRequired" : false,     "IsInvokedWhereAddressableValueRequired" : false,     "IsInvokedWhereICERequired" : false,     "IsAnyArgumentExpandedWhereModifiableValueRequired" : false,     "IsAnyArgumentExpandedWhereAddressableValueRequired" : false,     "IsAnyArgumentConditionallyEvaluated" : false,     "IsAnyArgumentNeverExpanded" : true,     "IsAnyArgumentNotAnExpression" : false  }
// Invocation	{     "Name" : "fuzz",     "DefinitionLocation" : "/maki/tests/has_same_name_as_other_declaration.c:33:9",     "InvocationLocation" : "/maki/tests/has_same_name_as_other_declaration.c:57:5",     "ASTKind" : "Expr",     "TypeSignature" : "int()",     "InvocationDepth" : 0,     "NumASTRoots" : 1,     "NumArguments" : 0,     "HasStringification" : false,     "HasTokenPasting" : false,     "HasAlignedArguments" : true,     "HasSameNameAsOtherDeclaration" : true,     "IsExpansionControlFlowStmt" : false,     "DoesBodyReferenceMacroDefinedAfterMacro" : false,     "DoesBodyReferenceDeclDeclaredAfterMacro" : false,     "DoesBodyContainDeclRefExpr" : false,     "DoesSubexpressionExpandedFromBodyHaveLocalType" : false,     "DoesSubexpressionExpandedFromBodyHaveTypeDefinedAfterMacro" : false,     "DoesAnyArgumentHaveSideEffects" : false,     "DoesAnyArgumentContainDeclRefExpr" : false,     "IsHygienic" : true,     "IsDefinitionLocationValid" : true,     "IsInvocationLocationValid" : true,     "IsObjectLike" : false,     "IsInvokedInMacroArgument" : false,     "IsNamePresentInCPPConditional" : false,     "IsExpansionICE" : true,     "IsExpansionTypeNull" : false,     "IsExpansionTypeAnonymous" : false,     "IsExpansionTypeLocalType" : false,     "IsExpansionTypeDefinedAfterMacro" : false,     "IsExpansionTypeVoid" : false,     "IsAnyArgumentTypeNull" : false,     "IsAnyArgumentTypeAnonymous" : true,     "IsAnyArgumentTypeLocalType" : false,     "IsAnyArgumentTypeDefinedAfterMacro" : false,     "IsAnyArgumentTypeVoid" : false,     "IsInvokedWhereModifiableValueRequired" : false,     "IsInvokedWhereAddressableValueRequired" : false,     "IsInvokedWhereICERequired" : false,     "IsAnyArgumentExpandedWhereModifiableValueRequired" : false,     "IsAnyArgumentExpandedWhereAddressableValueRequired" : false,     "IsAnyArgumentConditionallyEvaluated" : false,     "IsAnyArgumentNeverExpanded" : true,     "IsAnyArgumentNotAnExpression" : false  }
// Invocation	{     "Name" : "faz",     "DefinitionLocation" : "/maki/tests/has_same_name_as_other_declaration.c:37:9",     "InvocationLocation" : "/maki/tests/has_same_name_as_other_declaration.c:60:5",     "ASTKind" : "TypeLoc",     "TypeSignature" : "",     "InvocationDepth" : 0,     "NumASTRoots" : 1,     "NumArguments" : 0,     "HasStringification" : false,     "HasTokenPasting" : false,     "HasAlignedArguments" : true,     "HasSameNameAsOtherDeclaration" : true,     "IsExpansionControlFlowStmt" : false,     "DoesBodyReferenceMacroDefinedAfterMacro" : false,     "DoesBodyReferenceDeclDeclaredAfterMacro" : false,     "DoesBodyContainDeclRefExpr" : false,     "DoesSubexpressionExpandedFromBodyHaveLocalType" : false,     "DoesSubexpressionExpandedFromBodyHaveTypeDefinedAfterMacro" : false,     "DoesAnyArgumentHaveSideEffects" : false,     "DoesAnyArgumentContainDeclRefExpr" : false,     "IsHygienic" : true,     "IsDefinitionLocationValid" : true,     "IsInvocationLocationValid" : true,     "IsObjectLike" : true,     "IsInvokedInMacroArgument" : false,     "IsNamePresentInCPPConditional" : false,     "IsExpansionICE" : true,     "IsExpansionTypeNull" : false,     "IsExpansionTypeAnonymous" : false,     "IsExpansionTypeLocalType" : false,     "IsExpansionTypeDefinedAfterMacro" : false,     "IsExpansionTypeVoid" : false,     "IsAnyArgumentTypeNull" : false,     "IsAnyArgumentTypeAnonymous" : true,     "IsAnyArgumentTypeLocalType" : false,     "IsAnyArgumentTypeDefinedAfterMacro" : false,     "IsAnyArgumentTypeVoid" : false,     "IsInvokedWhereModifiableValueRequired" : false,     "IsInvokedWhereAddressableValueRequired" : false,     "IsInvokedWhereICERequired" : false,     "IsAnyArgumentExpandedWhereModifiableValueRequired" : false,     "IsAnyArgumentExpandedWhereAddressableValueRequired" : false,     "IsAnyArgumentConditionallyEvaluated" : false,     "IsAnyArgumentNeverExpanded" : true,     "IsAnyArgumentNotAnExpression" : false  }
