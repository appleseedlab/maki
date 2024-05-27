// RUN: maki %s -fplugin-arg-maki---no-system-macros -fplugin-arg-maki---no-builtin-macros -fplugin-arg-maki---no-invalid-macros | jq 'sort_by(.Kind, .DefinitionLocation, .InvocationLocation)' | FileCheck %s --color

// COM: Redefine strcpy to avoid including <string.h>
char *strcpy(char *d, const char *s) {
    return (char *)0;
}

// COM: This is the real test
#define bc_strcpy(d, l, s) strcpy(d, s)

int main(void) {
    char *s = (char *)0;
    bc_strcpy(s, 0, "abc");
    return 0;
}

// CHECK: [
// CHECK:   {
// CHECK:     "Kind": "Definition",
// CHECK:     "Name": "bc_strcpy",
// CHECK:     "IsObjectLike": false,
// CHECK:     "IsDefinitionLocationValid": true,
// CHECK:     "Body": "strcpy ( d , s )",
// CHECK:     "DefinitionLocation": "{{.*}}/Tests/bc_strcpy.c:9:9",
// CHECK:     "EndDefinitionLocation": "{{.*}}/Tests/bc_strcpy.c:9:39"
// CHECK:   },
// CHECK:   {
// CHECK:     "Kind": "Invocation",
// CHECK:     "Name": "bc_strcpy",
// CHECK:     "DefinitionLocation": "{{.*}}/Tests/bc_strcpy.c:9:9",
// CHECK:     "InvocationLocation": "{{.*}}/Tests/bc_strcpy.c:13:5",
// CHECK:     "ASTKind": "Expr",
// CHECK:     "TypeSignature": "char * bc_strcpy(char * d, , char * s)",
// CHECK:     "InvocationDepth": 0,
// CHECK:     "NumASTRoots": 1,
// CHECK:     "NumArguments": 3,
// CHECK:     "HasStringification": false,
// CHECK:     "HasTokenPasting": false,
// CHECK:     "HasAlignedArguments": true,
// CHECK:     "HasSameNameAsOtherDeclaration": false,
// CHECK:     "IsExpansionControlFlowStmt": false,
// CHECK:     "DoesBodyReferenceMacroDefinedAfterMacro": false,
// CHECK:     "DoesBodyReferenceDeclDeclaredAfterMacro": false,
// CHECK:     "DoesBodyContainDeclRefExpr": true,
// CHECK:     "DoesSubexpressionExpandedFromBodyHaveLocalType": false,
// CHECK:     "DoesSubexpressionExpandedFromBodyHaveTypeDefinedAfterMacro": false,
// CHECK:     "DoesAnyArgumentHaveSideEffects": false,
// CHECK:     "DoesAnyArgumentContainDeclRefExpr": true,
// CHECK:     "IsHygienic": true,
// CHECK:     "IsICERepresentableByInt32": false,
// CHECK:     "IsDefinitionLocationValid": true,
// CHECK:     "IsInvocationLocationValid": true,
// CHECK:     "IsObjectLike": false,
// CHECK:     "IsInvokedInMacroArgument": false,
// CHECK:     "IsNamePresentInCPPConditional": false,
// CHECK:     "IsExpansionICE": false,
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
// CHECK:     "IsInvokedWhereICERequired": false,
// CHECK:     "IsAnyArgumentExpandedWhereModifiableValueRequired": false,
// CHECK:     "IsAnyArgumentExpandedWhereAddressableValueRequired": false,
// CHECK:     "IsAnyArgumentConditionallyEvaluated": false,
// CHECK:     "IsAnyArgumentNeverExpanded": true,
// CHECK:     "IsAnyArgumentNotAnExpression": false
// CHECK:   }
// CHECK: ]