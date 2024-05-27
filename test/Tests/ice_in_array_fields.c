// RUN: maki %s -fplugin-arg-maki---no-system-macros -fplugin-arg-maki---no-builtin-macros -fplugin-arg-maki---no-invalid-macros | jq 'sort_by(.Kind, .DefinitionLocation, .InvocationLocation)' | FileCheck %s --color

#define LEN (3)

typedef struct foo {
    int array[LEN];
} foo;

typedef union bar {
    int array[LEN];
} bar;

int main(void) {
    return 0;
}

// CHECK: [
// CHECK:   {
// CHECK:     "Kind": "Definition",
// CHECK:     "Name": "LEN",
// CHECK:     "IsObjectLike": true,
// CHECK:     "IsDefinitionLocationValid": true,
// CHECK:     "Body": "( 3 )",
// CHECK:     "DefinitionLocation": "{{.*}}/Tests/ice_in_array_fields.c:3:9",
// CHECK:     "EndDefinitionLocation": "{{.*}}/Tests/ice_in_array_fields.c:3:15"
// CHECK:   },
// CHECK:   {
// CHECK:     "Kind": "Invocation",
// CHECK:     "Name": "LEN",
// CHECK:     "DefinitionLocation": "{{.*}}/Tests/ice_in_array_fields.c:3:9",
// CHECK:     "InvocationLocation": "{{.*}}/Tests/ice_in_array_fields.c:10:15",
// CHECK:     "ASTKind": "Expr",
// CHECK:     "TypeSignature": "int LEN",
// CHECK:     "InvocationDepth": 0,
// CHECK:     "NumASTRoots": 1,
// CHECK:     "NumArguments": 0,
// CHECK:     "HasStringification": false,
// CHECK:     "HasTokenPasting": false,
// CHECK:     "HasAlignedArguments": true,
// CHECK:     "HasSameNameAsOtherDeclaration": false,
// CHECK:     "IsExpansionControlFlowStmt": false,
// CHECK:     "DoesBodyReferenceMacroDefinedAfterMacro": false,
// CHECK:     "DoesBodyReferenceDeclDeclaredAfterMacro": false,
// CHECK:     "DoesBodyContainDeclRefExpr": false,
// CHECK:     "DoesSubexpressionExpandedFromBodyHaveLocalType": false,
// CHECK:     "DoesSubexpressionExpandedFromBodyHaveTypeDefinedAfterMacro": false,
// CHECK:     "DoesAnyArgumentHaveSideEffects": false,
// CHECK:     "DoesAnyArgumentContainDeclRefExpr": false,
// CHECK:     "IsHygienic": true,
// CHECK:     "IsICERepresentableByInt32": true,
// CHECK:     "IsDefinitionLocationValid": true,
// CHECK:     "IsInvocationLocationValid": true,
// CHECK:     "IsObjectLike": true,
// CHECK:     "IsInvokedInMacroArgument": false,
// CHECK:     "IsNamePresentInCPPConditional": false,
// CHECK:     "IsExpansionICE": true,
// CHECK:     "IsExpansionTypeNull": false,
// CHECK:     "IsExpansionTypeAnonymous": false,
// CHECK:     "IsExpansionTypeLocalType": false,
// CHECK:     "IsExpansionTypeDefinedAfterMacro": false,
// CHECK:     "IsExpansionTypeVoid": false,
// CHECK:     "IsAnyArgumentTypeNull": false,
// CHECK:     "IsAnyArgumentTypeAnonymous": false,
// CHECK:     "IsAnyArgumentTypeLocalType": false,
// CHECK:     "IsAnyArgumentTypeDefinedAfterMacro": false,
// CHECK:     "IsAnyArgumentTypeVoid": false,
// CHECK:     "IsInvokedWhereModifiableValueRequired": false,
// CHECK:     "IsInvokedWhereAddressableValueRequired": false,
// CHECK:     "IsInvokedWhereICERequired": true,
// CHECK:     "IsAnyArgumentExpandedWhereModifiableValueRequired": false,
// CHECK:     "IsAnyArgumentExpandedWhereAddressableValueRequired": false,
// CHECK:     "IsAnyArgumentConditionallyEvaluated": false,
// CHECK:     "IsAnyArgumentNeverExpanded": false,
// CHECK:     "IsAnyArgumentNotAnExpression": false
// CHECK:   },
// CHECK:   {
// CHECK:     "Kind": "Invocation",
// CHECK:     "Name": "LEN",
// CHECK:     "DefinitionLocation": "{{.*}}/Tests/ice_in_array_fields.c:3:9",
// CHECK:     "InvocationLocation": "{{.*}}/Tests/ice_in_array_fields.c:6:15",
// CHECK:     "ASTKind": "Expr",
// CHECK:     "TypeSignature": "int LEN",
// CHECK:     "InvocationDepth": 0,
// CHECK:     "NumASTRoots": 1,
// CHECK:     "NumArguments": 0,
// CHECK:     "HasStringification": false,
// CHECK:     "HasTokenPasting": false,
// CHECK:     "HasAlignedArguments": true,
// CHECK:     "HasSameNameAsOtherDeclaration": false,
// CHECK:     "IsExpansionControlFlowStmt": false,
// CHECK:     "DoesBodyReferenceMacroDefinedAfterMacro": false,
// CHECK:     "DoesBodyReferenceDeclDeclaredAfterMacro": false,
// CHECK:     "DoesBodyContainDeclRefExpr": false,
// CHECK:     "DoesSubexpressionExpandedFromBodyHaveLocalType": false,
// CHECK:     "DoesSubexpressionExpandedFromBodyHaveTypeDefinedAfterMacro": false,
// CHECK:     "DoesAnyArgumentHaveSideEffects": false,
// CHECK:     "DoesAnyArgumentContainDeclRefExpr": false,
// CHECK:     "IsHygienic": true,
// CHECK:     "IsICERepresentableByInt32": true,
// CHECK:     "IsDefinitionLocationValid": true,
// CHECK:     "IsInvocationLocationValid": true,
// CHECK:     "IsObjectLike": true,
// CHECK:     "IsInvokedInMacroArgument": false,
// CHECK:     "IsNamePresentInCPPConditional": false,
// CHECK:     "IsExpansionICE": true,
// CHECK:     "IsExpansionTypeNull": false,
// CHECK:     "IsExpansionTypeAnonymous": false,
// CHECK:     "IsExpansionTypeLocalType": false,
// CHECK:     "IsExpansionTypeDefinedAfterMacro": false,
// CHECK:     "IsExpansionTypeVoid": false,
// CHECK:     "IsAnyArgumentTypeNull": false,
// CHECK:     "IsAnyArgumentTypeAnonymous": false,
// CHECK:     "IsAnyArgumentTypeLocalType": false,
// CHECK:     "IsAnyArgumentTypeDefinedAfterMacro": false,
// CHECK:     "IsAnyArgumentTypeVoid": false,
// CHECK:     "IsInvokedWhereModifiableValueRequired": false,
// CHECK:     "IsInvokedWhereAddressableValueRequired": false,
// CHECK:     "IsInvokedWhereICERequired": true,
// CHECK:     "IsAnyArgumentExpandedWhereModifiableValueRequired": false,
// CHECK:     "IsAnyArgumentExpandedWhereAddressableValueRequired": false,
// CHECK:     "IsAnyArgumentConditionallyEvaluated": false,
// CHECK:     "IsAnyArgumentNeverExpanded": false,
// CHECK:     "IsAnyArgumentNotAnExpression": false
// CHECK:   }
// CHECK: ]