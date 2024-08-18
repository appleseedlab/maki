// RUN: maki %s -fplugin-arg-maki---no-system-macros -fplugin-arg-maki---no-builtin-macros -fplugin-arg-maki---no-invalid-macros | jq 'sort_by(.Kind, .DefinitionLocation, .InvocationLocation)' | FileCheck %s --color

// COM: Declare tag types before the macros that use them are defined, but don't
// COM: their underlying types until after the macros that use them are
// COM: defined.
typedef struct A A;
typedef union B B;
typedef enum C C;


// COM: Place B under another typedef to ensure that this check works
// COM: recursively.
typedef B B;

// COM: Place C under two typedefs to check recursive logic.
typedef C C;
typedef C C;

// COM: Also check that this checks works when a primitive type is typedef'd.
typedef int INT;

#define MACRO_ACCEPTING_A(a) ((a)->x)
#define MACRO_ACCEPTING_B(b) ((b)->x)
#define MACRO_ACCEPTING_C(c) ((c))
#define MACRO_ACCEPTING_INT(i) (i)

#define MACRO_WITH_A_IN_SUBEXPR() (1 + (A){.x = 1}.x)

struct A {
    int x;
};

union B {
    int x;
};

enum C { C_0 };

int main() {
    A a;
    B b;
    C c;

    MACRO_ACCEPTING_A(&a);
    MACRO_ACCEPTING_B(&b);
    MACRO_ACCEPTING_C(&c);
    MACRO_ACCEPTING_INT(1);

    MACRO_WITH_A_IN_SUBEXPR();

    return 0;
}

// CHECK: [
// CHECK:   {
// CHECK:     "Kind": "Definition",
// CHECK:     "Name": "MACRO_ACCEPTING_A",
// CHECK:     "IsObjectLike": false,
// CHECK:     "IsDefinitionLocationValid": true,
// CHECK:     "Body": "( ( a ) -> x )",
// CHECK:     "IsDefinedAtGlobalScope": true,
// CHECK:     "DefinitionLocation": "{{.*}}/Tests/typedef_type_declared_before_but_defined_after.c:22:9",
// CHECK:     "EndDefinitionLocation": "{{.*}}/Tests/typedef_type_declared_before_but_defined_after.c:22:38"
// CHECK:   },
// CHECK:   {
// CHECK:     "Kind": "Definition",
// CHECK:     "Name": "MACRO_ACCEPTING_B",
// CHECK:     "IsObjectLike": false,
// CHECK:     "IsDefinitionLocationValid": true,
// CHECK:     "Body": "( ( b ) -> x )",
// CHECK:     "IsDefinedAtGlobalScope": true,
// CHECK:     "DefinitionLocation": "{{.*}}/Tests/typedef_type_declared_before_but_defined_after.c:23:9",
// CHECK:     "EndDefinitionLocation": "{{.*}}/Tests/typedef_type_declared_before_but_defined_after.c:23:38"
// CHECK:   },
// CHECK:   {
// CHECK:     "Kind": "Definition",
// CHECK:     "Name": "MACRO_ACCEPTING_C",
// CHECK:     "IsObjectLike": false,
// CHECK:     "IsDefinitionLocationValid": true,
// CHECK:     "Body": "( ( c ) )",
// CHECK:     "IsDefinedAtGlobalScope": true,
// CHECK:     "DefinitionLocation": "{{.*}}/Tests/typedef_type_declared_before_but_defined_after.c:24:9",
// CHECK:     "EndDefinitionLocation": "{{.*}}/Tests/typedef_type_declared_before_but_defined_after.c:24:35"
// CHECK:   },
// CHECK:   {
// CHECK:     "Kind": "Definition",
// CHECK:     "Name": "MACRO_ACCEPTING_INT",
// CHECK:     "IsObjectLike": false,
// CHECK:     "IsDefinitionLocationValid": true,
// CHECK:     "Body": "( i )",
// CHECK:     "IsDefinedAtGlobalScope": true,
// CHECK:     "DefinitionLocation": "{{.*}}/Tests/typedef_type_declared_before_but_defined_after.c:25:9",
// CHECK:     "EndDefinitionLocation": "{{.*}}/Tests/typedef_type_declared_before_but_defined_after.c:25:35"
// CHECK:   },
// CHECK:   {
// CHECK:     "Kind": "Definition",
// CHECK:     "Name": "MACRO_WITH_A_IN_SUBEXPR",
// CHECK:     "IsObjectLike": false,
// CHECK:     "IsDefinitionLocationValid": true,
// CHECK:     "Body": "( 1 + ( A ) { . x = 1 } . x )",
// CHECK:     "IsDefinedAtGlobalScope": true,
// CHECK:     "DefinitionLocation": "{{.*}}/Tests/typedef_type_declared_before_but_defined_after.c:27:9",
// CHECK:     "EndDefinitionLocation": "{{.*}}/Tests/typedef_type_declared_before_but_defined_after.c:27:54"
// CHECK:   },
// CHECK:   {
// CHECK:     "Kind": "Invocation",
// CHECK:     "Name": "MACRO_ACCEPTING_A",
// CHECK:     "DefinitionLocation": "{{.*}}/Tests/typedef_type_declared_before_but_defined_after.c:22:9",
// CHECK:     "InvocationLocation": "{{.*}}/Tests/typedef_type_declared_before_but_defined_after.c:44:5",
// CHECK:     "ASTKind": "Expr",
// CHECK:     "TypeSignature": "int MACRO_ACCEPTING_A(struct A * a)",
// CHECK:     "InvocationDepth": 0,
// CHECK:     "NumASTRoots": 1,
// CHECK:     "NumArguments": 1,
// CHECK:     "HasStringification": false,
// CHECK:     "HasTokenPasting": false,
// CHECK:     "HasAlignedArguments": true,
// CHECK:     "HasSameNameAsOtherDeclaration": false,
// CHECK:     "IsExpansionControlFlowStmt": false,
// CHECK:     "DoesBodyReferenceMacroDefinedAfterMacro": false,
// CHECK:     "DoesBodyReferenceDeclDeclaredAfterMacro": false,
// CHECK:     "DoesBodyContainDeclRefExpr": false,
// CHECK:     "DoesSubexpressionExpandedFromBodyHaveLocalType": false,
// CHECK:     "DoesSubexpressionExpandedFromBodyHaveTypeDefinedAfterMacro": true,
// CHECK:     "DoesAnyArgumentHaveSideEffects": false,
// CHECK:     "DoesAnyArgumentContainDeclRefExpr": true,
// CHECK:     "IsHygienic": true,
// CHECK:     "IsICERepresentableByInt16": false,
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
// CHECK:     "IsExpansionTypeFunctionType": false,
// CHECK:     "IsAnyArgumentTypeNull": false,
// CHECK:     "IsAnyArgumentTypeAnonymous": false,
// CHECK:     "IsAnyArgumentTypeLocalType": false,
// CHECK:     "IsAnyArgumentTypeDefinedAfterMacro": true,
// CHECK:     "IsAnyArgumentTypeVoid": false,
// CHECK:     "IsAnyArgumentTypeFunctionType": false,
// CHECK:     "IsInvokedWhereModifiableValueRequired": false,
// CHECK:     "IsInvokedWhereAddressableValueRequired": false,
// CHECK:     "IsInvokedWhereICERequired": false,
// CHECK:     "IsInvokedWhereConstantExpressionRequired": false,
// CHECK:     "IsAnyArgumentExpandedWhereModifiableValueRequired": false,
// CHECK:     "IsAnyArgumentExpandedWhereAddressableValueRequired": false,
// CHECK:     "IsAnyArgumentConditionallyEvaluated": false,
// CHECK:     "IsAnyArgumentNeverExpanded": false,
// CHECK:     "IsAnyArgumentNotAnExpression": false
// CHECK:   },
// CHECK:   {
// CHECK:     "Kind": "Invocation",
// CHECK:     "Name": "MACRO_ACCEPTING_B",
// CHECK:     "DefinitionLocation": "{{.*}}/Tests/typedef_type_declared_before_but_defined_after.c:23:9",
// CHECK:     "InvocationLocation": "{{.*}}/Tests/typedef_type_declared_before_but_defined_after.c:45:5",
// CHECK:     "ASTKind": "Expr",
// CHECK:     "TypeSignature": "int MACRO_ACCEPTING_B(union B * b)",
// CHECK:     "InvocationDepth": 0,
// CHECK:     "NumASTRoots": 1,
// CHECK:     "NumArguments": 1,
// CHECK:     "HasStringification": false,
// CHECK:     "HasTokenPasting": false,
// CHECK:     "HasAlignedArguments": true,
// CHECK:     "HasSameNameAsOtherDeclaration": false,
// CHECK:     "IsExpansionControlFlowStmt": false,
// CHECK:     "DoesBodyReferenceMacroDefinedAfterMacro": false,
// CHECK:     "DoesBodyReferenceDeclDeclaredAfterMacro": false,
// CHECK:     "DoesBodyContainDeclRefExpr": false,
// CHECK:     "DoesSubexpressionExpandedFromBodyHaveLocalType": false,
// CHECK:     "DoesSubexpressionExpandedFromBodyHaveTypeDefinedAfterMacro": true,
// CHECK:     "DoesAnyArgumentHaveSideEffects": false,
// CHECK:     "DoesAnyArgumentContainDeclRefExpr": true,
// CHECK:     "IsHygienic": true,
// CHECK:     "IsICERepresentableByInt16": false,
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
// CHECK:     "IsExpansionTypeFunctionType": false,
// CHECK:     "IsAnyArgumentTypeNull": false,
// CHECK:     "IsAnyArgumentTypeAnonymous": false,
// CHECK:     "IsAnyArgumentTypeLocalType": false,
// CHECK:     "IsAnyArgumentTypeDefinedAfterMacro": true,
// CHECK:     "IsAnyArgumentTypeVoid": false,
// CHECK:     "IsAnyArgumentTypeFunctionType": false,
// CHECK:     "IsInvokedWhereModifiableValueRequired": false,
// CHECK:     "IsInvokedWhereAddressableValueRequired": false,
// CHECK:     "IsInvokedWhereICERequired": false,
// CHECK:     "IsInvokedWhereConstantExpressionRequired": false,
// CHECK:     "IsAnyArgumentExpandedWhereModifiableValueRequired": false,
// CHECK:     "IsAnyArgumentExpandedWhereAddressableValueRequired": false,
// CHECK:     "IsAnyArgumentConditionallyEvaluated": false,
// CHECK:     "IsAnyArgumentNeverExpanded": false,
// CHECK:     "IsAnyArgumentNotAnExpression": false
// CHECK:   },
// CHECK:   {
// CHECK:     "Kind": "Invocation",
// CHECK:     "Name": "MACRO_ACCEPTING_C",
// CHECK:     "DefinitionLocation": "{{.*}}/Tests/typedef_type_declared_before_but_defined_after.c:24:9",
// CHECK:     "InvocationLocation": "{{.*}}/Tests/typedef_type_declared_before_but_defined_after.c:46:5",
// CHECK:     "ASTKind": "Expr",
// CHECK:     "TypeSignature": "enum C * MACRO_ACCEPTING_C(enum C * c)",
// CHECK:     "InvocationDepth": 0,
// CHECK:     "NumASTRoots": 1,
// CHECK:     "NumArguments": 1,
// CHECK:     "HasStringification": false,
// CHECK:     "HasTokenPasting": false,
// CHECK:     "HasAlignedArguments": true,
// CHECK:     "HasSameNameAsOtherDeclaration": false,
// CHECK:     "IsExpansionControlFlowStmt": false,
// CHECK:     "DoesBodyReferenceMacroDefinedAfterMacro": false,
// CHECK:     "DoesBodyReferenceDeclDeclaredAfterMacro": false,
// CHECK:     "DoesBodyContainDeclRefExpr": false,
// CHECK:     "DoesSubexpressionExpandedFromBodyHaveLocalType": false,
// CHECK:     "DoesSubexpressionExpandedFromBodyHaveTypeDefinedAfterMacro": true,
// CHECK:     "DoesAnyArgumentHaveSideEffects": false,
// CHECK:     "DoesAnyArgumentContainDeclRefExpr": true,
// CHECK:     "IsHygienic": true,
// CHECK:     "IsICERepresentableByInt16": false,
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
// CHECK:     "IsExpansionTypeDefinedAfterMacro": true,
// CHECK:     "IsExpansionTypeVoid": false,
// CHECK:     "IsExpansionTypeFunctionType": false,
// CHECK:     "IsAnyArgumentTypeNull": false,
// CHECK:     "IsAnyArgumentTypeAnonymous": false,
// CHECK:     "IsAnyArgumentTypeLocalType": false,
// CHECK:     "IsAnyArgumentTypeDefinedAfterMacro": true,
// CHECK:     "IsAnyArgumentTypeVoid": false,
// CHECK:     "IsAnyArgumentTypeFunctionType": false,
// CHECK:     "IsInvokedWhereModifiableValueRequired": false,
// CHECK:     "IsInvokedWhereAddressableValueRequired": false,
// CHECK:     "IsInvokedWhereICERequired": false,
// CHECK:     "IsInvokedWhereConstantExpressionRequired": false,
// CHECK:     "IsAnyArgumentExpandedWhereModifiableValueRequired": false,
// CHECK:     "IsAnyArgumentExpandedWhereAddressableValueRequired": false,
// CHECK:     "IsAnyArgumentConditionallyEvaluated": false,
// CHECK:     "IsAnyArgumentNeverExpanded": false,
// CHECK:     "IsAnyArgumentNotAnExpression": false
// CHECK:   },
// CHECK:   {
// CHECK:     "Kind": "Invocation",
// CHECK:     "Name": "MACRO_ACCEPTING_INT",
// CHECK:     "DefinitionLocation": "{{.*}}/Tests/typedef_type_declared_before_but_defined_after.c:25:9",
// CHECK:     "InvocationLocation": "{{.*}}/Tests/typedef_type_declared_before_but_defined_after.c:47:5",
// CHECK:     "ASTKind": "Expr",
// CHECK:     "TypeSignature": "int MACRO_ACCEPTING_INT(int i)",
// CHECK:     "InvocationDepth": 0,
// CHECK:     "NumASTRoots": 1,
// CHECK:     "NumArguments": 1,
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
// CHECK:     "IsICERepresentableByInt16": true,
// CHECK:     "IsICERepresentableByInt32": true,
// CHECK:     "IsDefinitionLocationValid": true,
// CHECK:     "IsInvocationLocationValid": true,
// CHECK:     "IsObjectLike": false,
// CHECK:     "IsInvokedInMacroArgument": false,
// CHECK:     "IsNamePresentInCPPConditional": false,
// CHECK:     "IsExpansionICE": true,
// CHECK:     "IsExpansionTypeNull": false,
// CHECK:     "IsExpansionTypeAnonymous": false,
// CHECK:     "IsExpansionTypeLocalType": false,
// CHECK:     "IsExpansionTypeDefinedAfterMacro": false,
// CHECK:     "IsExpansionTypeVoid": false,
// CHECK:     "IsExpansionTypeFunctionType": false,
// CHECK:     "IsAnyArgumentTypeNull": false,
// CHECK:     "IsAnyArgumentTypeAnonymous": false,
// CHECK:     "IsAnyArgumentTypeLocalType": false,
// CHECK:     "IsAnyArgumentTypeDefinedAfterMacro": false,
// CHECK:     "IsAnyArgumentTypeVoid": false,
// CHECK:     "IsAnyArgumentTypeFunctionType": false,
// CHECK:     "IsInvokedWhereModifiableValueRequired": false,
// CHECK:     "IsInvokedWhereAddressableValueRequired": false,
// CHECK:     "IsInvokedWhereICERequired": false,
// CHECK:     "IsInvokedWhereConstantExpressionRequired": false,
// CHECK:     "IsAnyArgumentExpandedWhereModifiableValueRequired": false,
// CHECK:     "IsAnyArgumentExpandedWhereAddressableValueRequired": false,
// CHECK:     "IsAnyArgumentConditionallyEvaluated": false,
// CHECK:     "IsAnyArgumentNeverExpanded": false,
// CHECK:     "IsAnyArgumentNotAnExpression": false
// CHECK:   },
// CHECK:   {
// CHECK:     "Kind": "Invocation",
// CHECK:     "Name": "MACRO_WITH_A_IN_SUBEXPR",
// CHECK:     "DefinitionLocation": "{{.*}}/Tests/typedef_type_declared_before_but_defined_after.c:27:9",
// CHECK:     "InvocationLocation": "{{.*}}/Tests/typedef_type_declared_before_but_defined_after.c:49:5",
// CHECK:     "ASTKind": "Expr",
// CHECK:     "TypeSignature": "int MACRO_WITH_A_IN_SUBEXPR()",
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
// CHECK:     "DoesSubexpressionExpandedFromBodyHaveTypeDefinedAfterMacro": true,
// CHECK:     "DoesAnyArgumentHaveSideEffects": false,
// CHECK:     "DoesAnyArgumentContainDeclRefExpr": false,
// CHECK:     "IsHygienic": true,
// CHECK:     "IsICERepresentableByInt16": false,
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
// CHECK:     "IsExpansionTypeFunctionType": false,
// CHECK:     "IsAnyArgumentTypeNull": false,
// CHECK:     "IsAnyArgumentTypeAnonymous": false,
// CHECK:     "IsAnyArgumentTypeLocalType": false,
// CHECK:     "IsAnyArgumentTypeDefinedAfterMacro": false,
// CHECK:     "IsAnyArgumentTypeVoid": false,
// CHECK:     "IsAnyArgumentTypeFunctionType": false,
// CHECK:     "IsInvokedWhereModifiableValueRequired": false,
// CHECK:     "IsInvokedWhereAddressableValueRequired": false,
// CHECK:     "IsInvokedWhereICERequired": false,
// CHECK:     "IsInvokedWhereConstantExpressionRequired": false,
// CHECK:     "IsAnyArgumentExpandedWhereModifiableValueRequired": false,
// CHECK:     "IsAnyArgumentExpandedWhereAddressableValueRequired": false,
// CHECK:     "IsAnyArgumentConditionallyEvaluated": false,
// CHECK:     "IsAnyArgumentNeverExpanded": false,
// CHECK:     "IsAnyArgumentNotAnExpression": false
// CHECK:   }
// CHECK: ]