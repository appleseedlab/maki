// RUN: maki %s -fplugin-arg-maki---no-system-macros -fplugin-arg-maki---no-builtin-macros -fplugin-arg-maki---no-invalid-macros | jq 'sort_by(.Kind, .DefinitionLocation, .InvocationLocation, .Name)' | FileCheck %s --color

#define SIZE_OF_MY_STRUCT() (sizeof(struct my_struct))
#define SIZE_OF_MY_TYPEDEFD_STRUCT() (sizeof(my_typedefd_struct))

#define CAST_TO_MY_STRUCT(X) ((struct my_struct) X)
#define CAST_TO_MY_TYPEDEFD_STRUCT(X) ((my_typedefd_struct) X)

int main(void) {
    struct my_struct {
        int x;
    };

    typedef struct my_typedefd_struct {
        int y;
    } my_typedefd_struct;

    SIZE_OF_MY_STRUCT();
    SIZE_OF_MY_TYPEDEFD_STRUCT();

    CAST_TO_MY_STRUCT({});
    CAST_TO_MY_TYPEDEFD_STRUCT({});

    return 0;
}

// CHECK: [
// CHECK:   {
// CHECK:     "Kind": "Definition",
// CHECK:     "Name": "SIZE_OF_MY_STRUCT",
// CHECK:     "IsObjectLike": false,
// CHECK:     "IsDefinitionLocationValid": true,
// CHECK:     "Body": "( sizeof ( struct my_struct ) )",
// CHECK:     "IsDefinedAtGlobalScope": true,
// CHECK:     "DefinitionLocation": "{{.*}}/Tests/sizeof_and_cast_local_type.c:3:9",
// CHECK:     "EndDefinitionLocation": "{{.*}}/Tests/sizeof_and_cast_local_type.c:3:54"
// CHECK:   },
// CHECK:   {
// CHECK:     "Kind": "Definition",
// CHECK:     "Name": "SIZE_OF_MY_TYPEDEFD_STRUCT",
// CHECK:     "IsObjectLike": false,
// CHECK:     "IsDefinitionLocationValid": true,
// CHECK:     "Body": "( sizeof ( my_typedefd_struct ) )",
// CHECK:     "IsDefinedAtGlobalScope": true,
// CHECK:     "DefinitionLocation": "{{.*}}/Tests/sizeof_and_cast_local_type.c:4:9",
// CHECK:     "EndDefinitionLocation": "{{.*}}/Tests/sizeof_and_cast_local_type.c:4:65"
// CHECK:   },
// CHECK:   {
// CHECK:     "Kind": "Definition",
// CHECK:     "Name": "CAST_TO_MY_STRUCT",
// CHECK:     "IsObjectLike": false,
// CHECK:     "IsDefinitionLocationValid": true,
// CHECK:     "Body": "( ( struct my_struct ) X )",
// CHECK:     "IsDefinedAtGlobalScope": true,
// CHECK:     "DefinitionLocation": "{{.*}}/Tests/sizeof_and_cast_local_type.c:6:9",
// CHECK:     "EndDefinitionLocation": "{{.*}}/Tests/sizeof_and_cast_local_type.c:6:51"
// CHECK:   },
// CHECK:   {
// CHECK:     "Kind": "Definition",
// CHECK:     "Name": "CAST_TO_MY_TYPEDEFD_STRUCT",
// CHECK:     "IsObjectLike": false,
// CHECK:     "IsDefinitionLocationValid": true,
// CHECK:     "Body": "( ( my_typedefd_struct ) X )",
// CHECK:     "IsDefinedAtGlobalScope": true,
// CHECK:     "DefinitionLocation": "{{.*}}/Tests/sizeof_and_cast_local_type.c:7:9",
// CHECK:     "EndDefinitionLocation": "{{.*}}/Tests/sizeof_and_cast_local_type.c:7:62"
// CHECK:   },
// CHECK:   {
// CHECK:     "Kind": "Invocation",
// CHECK:     "Name": "SIZE_OF_MY_STRUCT",
// CHECK:     "DefinitionLocation": "{{.*}}/Tests/sizeof_and_cast_local_type.c:3:9",
// CHECK:     "InvocationLocation": "{{.*}}/Tests/sizeof_and_cast_local_type.c:18:5",
// CHECK:     "ASTKind": "Expr",
// CHECK:     "TypeSignature": "unsigned long SIZE_OF_MY_STRUCT()",
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
// CHECK:     "DoesSubexpressionExpandedFromBodyHaveLocalType": true,
// CHECK:     "DoesSubexpressionExpandedFromBodyHaveTypeDefinedAfterMacro": true,
// CHECK:     "DoesAnyArgumentHaveSideEffects": false,
// CHECK:     "DoesAnyArgumentContainDeclRefExpr": false,
// CHECK:     "IsHygienic": true,
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
// CHECK:     "IsAnyArgumentTypeNull": false,
// CHECK:     "IsAnyArgumentTypeAnonymous": false,
// CHECK:     "IsAnyArgumentTypeLocalType": false,
// CHECK:     "IsAnyArgumentTypeDefinedAfterMacro": false,
// CHECK:     "IsAnyArgumentTypeVoid": false,
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
// CHECK:     "Name": "SIZE_OF_MY_TYPEDEFD_STRUCT",
// CHECK:     "DefinitionLocation": "{{.*}}/Tests/sizeof_and_cast_local_type.c:4:9",
// CHECK:     "InvocationLocation": "{{.*}}/Tests/sizeof_and_cast_local_type.c:19:5",
// CHECK:     "ASTKind": "Expr",
// CHECK:     "TypeSignature": "unsigned long SIZE_OF_MY_TYPEDEFD_STRUCT()",
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
// CHECK:     "DoesSubexpressionExpandedFromBodyHaveLocalType": true,
// CHECK:     "DoesSubexpressionExpandedFromBodyHaveTypeDefinedAfterMacro": true,
// CHECK:     "DoesAnyArgumentHaveSideEffects": false,
// CHECK:     "DoesAnyArgumentContainDeclRefExpr": false,
// CHECK:     "IsHygienic": true,
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
// CHECK:     "IsAnyArgumentTypeNull": false,
// CHECK:     "IsAnyArgumentTypeAnonymous": false,
// CHECK:     "IsAnyArgumentTypeLocalType": false,
// CHECK:     "IsAnyArgumentTypeDefinedAfterMacro": false,
// CHECK:     "IsAnyArgumentTypeVoid": false,
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
// CHECK:     "Name": "CAST_TO_MY_STRUCT",
// CHECK:     "DefinitionLocation": "{{.*}}/Tests/sizeof_and_cast_local_type.c:6:9",
// CHECK:     "InvocationLocation": "{{.*}}/Tests/sizeof_and_cast_local_type.c:21:5",
// CHECK:     "ASTKind": "Expr",
// CHECK:     "TypeSignature": "struct my_struct CAST_TO_MY_STRUCT(struct my_struct X)",
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
// CHECK:     "DoesSubexpressionExpandedFromBodyHaveLocalType": true,
// CHECK:     "DoesSubexpressionExpandedFromBodyHaveTypeDefinedAfterMacro": true,
// CHECK:     "DoesAnyArgumentHaveSideEffects": false,
// CHECK:     "DoesAnyArgumentContainDeclRefExpr": false,
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
// CHECK:     "IsExpansionTypeLocalType": true,
// CHECK:     "IsExpansionTypeDefinedAfterMacro": true,
// CHECK:     "IsExpansionTypeVoid": false,
// CHECK:     "IsAnyArgumentTypeNull": false,
// CHECK:     "IsAnyArgumentTypeAnonymous": false,
// CHECK:     "IsAnyArgumentTypeLocalType": true,
// CHECK:     "IsAnyArgumentTypeDefinedAfterMacro": true,
// CHECK:     "IsAnyArgumentTypeVoid": false,
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
// CHECK:     "Name": "CAST_TO_MY_TYPEDEFD_STRUCT",
// CHECK:     "DefinitionLocation": "{{.*}}/Tests/sizeof_and_cast_local_type.c:7:9",
// CHECK:     "InvocationLocation": "{{.*}}/Tests/sizeof_and_cast_local_type.c:22:5",
// CHECK:     "ASTKind": "Expr",
// CHECK:     "TypeSignature": "struct my_typedefd_struct CAST_TO_MY_TYPEDEFD_STRUCT(struct my_typedefd_struct X)",
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
// CHECK:     "DoesSubexpressionExpandedFromBodyHaveLocalType": true,
// CHECK:     "DoesSubexpressionExpandedFromBodyHaveTypeDefinedAfterMacro": true,
// CHECK:     "DoesAnyArgumentHaveSideEffects": false,
// CHECK:     "DoesAnyArgumentContainDeclRefExpr": false,
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
// CHECK:     "IsExpansionTypeLocalType": true,
// CHECK:     "IsExpansionTypeDefinedAfterMacro": true,
// CHECK:     "IsExpansionTypeVoid": false,
// CHECK:     "IsAnyArgumentTypeNull": false,
// CHECK:     "IsAnyArgumentTypeAnonymous": false,
// CHECK:     "IsAnyArgumentTypeLocalType": true,
// CHECK:     "IsAnyArgumentTypeDefinedAfterMacro": true,
// CHECK:     "IsAnyArgumentTypeVoid": false,
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
