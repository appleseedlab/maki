// RUN: maki %s -fplugin-arg-maki---no-system-macros -fplugin-arg-maki---no-builtin-macros -fplugin-arg-maki---no-invalid-macros | jq 'sort_by(.Kind, .DefinitionLocation, .InvocationLocation, .Name)' | FileCheck %s --color
#define BITS_PER_UNIT (8)
#define MAX_BITSIZE_MODE_ANY_INT (64 * BITS_PER_UNIT)
#define HOST_BITS_PER_WIDE_INT 64
#define WIDE_INT_MAX_ELTS                                  \
    ((MAX_BITSIZE_MODE_ANY_INT + HOST_BITS_PER_WIDE_INT) / \
     HOST_BITS_PER_WIDE_INT)
#define WIDE_INT_MAX_PRECISION (WIDE_INT_MAX_ELTS * HOST_BITS_PER_WIDE_INT)
template <typename T> class generic_wide_int;
template <int N> class fixed_wide_int_storage;
#define FIXED_WIDE_INT(N) generic_wide_int<fixed_wide_int_storage<N>>
typedef FIXED_WIDE_INT(WIDE_INT_MAX_PRECISION) widest_int;
#define GTY(x)
#define HOST_WIDE_INT long
template <bool SE, bool HDP = true> class wide_int_ref_storage;
typedef generic_wide_int<wide_int_ref_storage<false>> wide_int_ref;
enum signop { SIGNED, UNSIGNED };
template <int N> class GTY(()) fixed_wide_int_storage {
private:
    HOST_WIDE_INT val[(N + HOST_BITS_PER_WIDE_INT + 1) / HOST_BITS_PER_WIDE_INT];
    unsigned int len;

public:
    fixed_wide_int_storage();
    template <typename T> fixed_wide_int_storage(const T &);
    unsigned int get_precision() const;
    const HOST_WIDE_INT *get_val() const;
    unsigned int get_len() const;
    HOST_WIDE_INT *write_val();
    void set_len(unsigned int, bool = false);
    static FIXED_WIDE_INT(N) from(const wide_int_ref &, signop);
    static FIXED_WIDE_INT(N)
        from_array(const HOST_WIDE_INT *, unsigned int, bool = true);
};
int main(void) {
    return 0;
}

// CHECK: [
// CHECK:   {
// CHECK:     "Kind": "Definition",
// CHECK:     "Name": "FIXED_WIDE_INT",
// CHECK:     "IsObjectLike": false,
// CHECK:     "IsDefinitionLocationValid": true,
// CHECK:     "Body": "generic_wide_int < fixed_wide_int_storage < N >>",
// CHECK:     "IsDefinedAtGlobalScope": true,
// CHECK:     "DefinitionLocation": "{{.*}}/Tests/wide-int.cc:11:9",
// CHECK:     "EndDefinitionLocation": "{{.*}}/Tests/wide-int.cc:11:68"
// CHECK:   },
// CHECK:   {
// CHECK:     "Kind": "Definition",
// CHECK:     "Name": "GTY",
// CHECK:     "IsObjectLike": false,
// CHECK:     "IsDefinitionLocationValid": true,
// CHECK:     "Body": "",
// CHECK:     "IsDefinedAtGlobalScope": true,
// CHECK:     "DefinitionLocation": "{{.*}}/Tests/wide-int.cc:13:9",
// CHECK:     "EndDefinitionLocation": "{{.*}}/Tests/wide-int.cc:13:14"
// CHECK:   },
// CHECK:   {
// CHECK:     "Kind": "Definition",
// CHECK:     "Name": "HOST_WIDE_INT",
// CHECK:     "IsObjectLike": true,
// CHECK:     "IsDefinitionLocationValid": true,
// CHECK:     "Body": "long",
// CHECK:     "IsDefinedAtGlobalScope": true,
// CHECK:     "DefinitionLocation": "{{.*}}/Tests/wide-int.cc:14:9",
// CHECK:     "EndDefinitionLocation": "{{.*}}/Tests/wide-int.cc:14:23"
// CHECK:   },
// CHECK:   {
// CHECK:     "Kind": "Definition",
// CHECK:     "Name": "BITS_PER_UNIT",
// CHECK:     "IsObjectLike": true,
// CHECK:     "IsDefinitionLocationValid": true,
// CHECK:     "Body": "( 8 )",
// CHECK:     "IsDefinedAtGlobalScope": true,
// CHECK:     "DefinitionLocation": "{{.*}}/Tests/wide-int.cc:2:9",
// CHECK:     "EndDefinitionLocation": "{{.*}}/Tests/wide-int.cc:2:25"
// CHECK:   },
// CHECK:   {
// CHECK:     "Kind": "Definition",
// CHECK:     "Name": "MAX_BITSIZE_MODE_ANY_INT",
// CHECK:     "IsObjectLike": true,
// CHECK:     "IsDefinitionLocationValid": true,
// CHECK:     "Body": "( 64 * BITS_PER_UNIT )",
// CHECK:     "IsDefinedAtGlobalScope": true,
// CHECK:     "DefinitionLocation": "{{.*}}/Tests/wide-int.cc:3:9",
// CHECK:     "EndDefinitionLocation": "{{.*}}/Tests/wide-int.cc:3:53"
// CHECK:   },
// CHECK:   {
// CHECK:     "Kind": "Definition",
// CHECK:     "Name": "HOST_BITS_PER_WIDE_INT",
// CHECK:     "IsObjectLike": true,
// CHECK:     "IsDefinitionLocationValid": true,
// CHECK:     "Body": "64",
// CHECK:     "IsDefinedAtGlobalScope": true,
// CHECK:     "DefinitionLocation": "{{.*}}/Tests/wide-int.cc:4:9",
// CHECK:     "EndDefinitionLocation": "{{.*}}/Tests/wide-int.cc:4:32"
// CHECK:   },
// CHECK:   {
// CHECK:     "Kind": "Definition",
// CHECK:     "Name": "WIDE_INT_MAX_ELTS",
// CHECK:     "IsObjectLike": true,
// CHECK:     "IsDefinitionLocationValid": true,
// CHECK:     "Body": "( ( MAX_BITSIZE_MODE_ANY_INT + HOST_BITS_PER_WIDE_INT ) / HOST_BITS_PER_WIDE_INT )",
// CHECK:     "IsDefinedAtGlobalScope": true,
// CHECK:     "DefinitionLocation": "{{.*}}/Tests/wide-int.cc:5:9",
// CHECK:     "EndDefinitionLocation": "{{.*}}/Tests/wide-int.cc:7:28"
// CHECK:   },
// CHECK:   {
// CHECK:     "Kind": "Definition",
// CHECK:     "Name": "WIDE_INT_MAX_PRECISION",
// CHECK:     "IsObjectLike": true,
// CHECK:     "IsDefinitionLocationValid": true,
// CHECK:     "Body": "( WIDE_INT_MAX_ELTS * HOST_BITS_PER_WIDE_INT )",
// CHECK:     "IsDefinedAtGlobalScope": true,
// CHECK:     "DefinitionLocation": "{{.*}}/Tests/wide-int.cc:8:9",
// CHECK:     "EndDefinitionLocation": "{{.*}}/Tests/wide-int.cc:8:75"
// CHECK:   },
// CHECK:   {
// CHECK:     "Kind": "Invocation",
// CHECK:     "Name": "FIXED_WIDE_INT",
// CHECK:     "DefinitionLocation": "{{.*}}/Tests/wide-int.cc:11:9",
// CHECK:     "InvocationLocation": "{{.*}}/Tests/wide-int.cc:12:9",
// CHECK:     "ASTKind": "",
// CHECK:     "TypeSignature": "",
// CHECK:     "InvocationDepth": 0,
// CHECK:     "NumASTRoots": 0,
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
// CHECK:     "IsHygienic": false,
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
// CHECK:     "IsInvokedWhereConstantExpressionRequired": false,
// CHECK:     "IsAnyArgumentExpandedWhereModifiableValueRequired": false,
// CHECK:     "IsAnyArgumentExpandedWhereAddressableValueRequired": false,
// CHECK:     "IsAnyArgumentConditionallyEvaluated": false,
// CHECK:     "IsAnyArgumentNeverExpanded": false,
// CHECK:     "IsAnyArgumentNotAnExpression": false
// CHECK:   },
// CHECK:   {
// CHECK:     "Kind": "Invocation",
// CHECK:     "Name": "FIXED_WIDE_INT",
// CHECK:     "DefinitionLocation": "{{.*}}/Tests/wide-int.cc:11:9",
// CHECK:     "InvocationLocation": "{{.*}}/Tests/wide-int.cc:31:12",
// CHECK:     "ASTKind": "",
// CHECK:     "TypeSignature": "",
// CHECK:     "InvocationDepth": 0,
// CHECK:     "NumASTRoots": 0,
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
// CHECK:     "DoesAnyArgumentContainDeclRefExpr": true,
// CHECK:     "IsHygienic": false,
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
// CHECK:     "IsInvokedWhereConstantExpressionRequired": false,
// CHECK:     "IsAnyArgumentExpandedWhereModifiableValueRequired": false,
// CHECK:     "IsAnyArgumentExpandedWhereAddressableValueRequired": false,
// CHECK:     "IsAnyArgumentConditionallyEvaluated": false,
// CHECK:     "IsAnyArgumentNeverExpanded": false,
// CHECK:     "IsAnyArgumentNotAnExpression": false
// CHECK:   },
// CHECK:   {
// CHECK:     "Kind": "Invocation",
// CHECK:     "Name": "FIXED_WIDE_INT",
// CHECK:     "DefinitionLocation": "{{.*}}/Tests/wide-int.cc:11:9",
// CHECK:     "InvocationLocation": "{{.*}}/Tests/wide-int.cc:32:12",
// CHECK:     "ASTKind": "",
// CHECK:     "TypeSignature": "",
// CHECK:     "InvocationDepth": 0,
// CHECK:     "NumASTRoots": 0,
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
// CHECK:     "DoesAnyArgumentContainDeclRefExpr": true,
// CHECK:     "IsHygienic": false,
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
// CHECK:     "IsInvokedWhereConstantExpressionRequired": false,
// CHECK:     "IsAnyArgumentExpandedWhereModifiableValueRequired": false,
// CHECK:     "IsAnyArgumentExpandedWhereAddressableValueRequired": false,
// CHECK:     "IsAnyArgumentConditionallyEvaluated": false,
// CHECK:     "IsAnyArgumentNeverExpanded": false,
// CHECK:     "IsAnyArgumentNotAnExpression": false
// CHECK:   },
// CHECK:   {
// CHECK:     "Kind": "Invocation",
// CHECK:     "Name": "GTY",
// CHECK:     "DefinitionLocation": "{{.*}}/Tests/wide-int.cc:13:9",
// CHECK:     "InvocationLocation": "{{.*}}/Tests/wide-int.cc:18:24",
// CHECK:     "ASTKind": "",
// CHECK:     "TypeSignature": "",
// CHECK:     "InvocationDepth": 0,
// CHECK:     "NumASTRoots": 0,
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
// CHECK:     "IsHygienic": false,
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
// CHECK:     "IsInvokedWhereConstantExpressionRequired": false,
// CHECK:     "IsAnyArgumentExpandedWhereModifiableValueRequired": false,
// CHECK:     "IsAnyArgumentExpandedWhereAddressableValueRequired": false,
// CHECK:     "IsAnyArgumentConditionallyEvaluated": false,
// CHECK:     "IsAnyArgumentNeverExpanded": false,
// CHECK:     "IsAnyArgumentNotAnExpression": false
// CHECK:   },
// CHECK:   {
// CHECK:     "Kind": "Invocation",
// CHECK:     "Name": "HOST_WIDE_INT",
// CHECK:     "DefinitionLocation": "{{.*}}/Tests/wide-int.cc:14:9",
// CHECK:     "InvocationLocation": "{{.*}}/Tests/wide-int.cc:20:5",
// CHECK:     "ASTKind": "TypeLoc",
// CHECK:     "TypeSignature": "",
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
// CHECK:     "IsHygienic": false,
// CHECK:     "IsICERepresentableByInt32": false,
// CHECK:     "IsDefinitionLocationValid": true,
// CHECK:     "IsInvocationLocationValid": true,
// CHECK:     "IsObjectLike": true,
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
// CHECK:     "IsInvokedWhereConstantExpressionRequired": false,
// CHECK:     "IsAnyArgumentExpandedWhereModifiableValueRequired": false,
// CHECK:     "IsAnyArgumentExpandedWhereAddressableValueRequired": false,
// CHECK:     "IsAnyArgumentConditionallyEvaluated": false,
// CHECK:     "IsAnyArgumentNeverExpanded": false,
// CHECK:     "IsAnyArgumentNotAnExpression": false
// CHECK:   },
// CHECK:   {
// CHECK:     "Kind": "Invocation",
// CHECK:     "Name": "HOST_WIDE_INT",
// CHECK:     "DefinitionLocation": "{{.*}}/Tests/wide-int.cc:14:9",
// CHECK:     "InvocationLocation": "{{.*}}/Tests/wide-int.cc:27:11",
// CHECK:     "ASTKind": "TypeLoc",
// CHECK:     "TypeSignature": "",
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
// CHECK:     "IsHygienic": false,
// CHECK:     "IsICERepresentableByInt32": false,
// CHECK:     "IsDefinitionLocationValid": true,
// CHECK:     "IsInvocationLocationValid": true,
// CHECK:     "IsObjectLike": true,
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
// CHECK:     "IsInvokedWhereConstantExpressionRequired": false,
// CHECK:     "IsAnyArgumentExpandedWhereModifiableValueRequired": false,
// CHECK:     "IsAnyArgumentExpandedWhereAddressableValueRequired": false,
// CHECK:     "IsAnyArgumentConditionallyEvaluated": false,
// CHECK:     "IsAnyArgumentNeverExpanded": false,
// CHECK:     "IsAnyArgumentNotAnExpression": false
// CHECK:   },
// CHECK:   {
// CHECK:     "Kind": "Invocation",
// CHECK:     "Name": "HOST_WIDE_INT",
// CHECK:     "DefinitionLocation": "{{.*}}/Tests/wide-int.cc:14:9",
// CHECK:     "InvocationLocation": "{{.*}}/Tests/wide-int.cc:29:5",
// CHECK:     "ASTKind": "TypeLoc",
// CHECK:     "TypeSignature": "",
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
// CHECK:     "IsHygienic": false,
// CHECK:     "IsICERepresentableByInt32": false,
// CHECK:     "IsDefinitionLocationValid": true,
// CHECK:     "IsInvocationLocationValid": true,
// CHECK:     "IsObjectLike": true,
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
// CHECK:     "IsInvokedWhereConstantExpressionRequired": false,
// CHECK:     "IsAnyArgumentExpandedWhereModifiableValueRequired": false,
// CHECK:     "IsAnyArgumentExpandedWhereAddressableValueRequired": false,
// CHECK:     "IsAnyArgumentConditionallyEvaluated": false,
// CHECK:     "IsAnyArgumentNeverExpanded": false,
// CHECK:     "IsAnyArgumentNotAnExpression": false
// CHECK:   },
// CHECK:   {
// CHECK:     "Kind": "Invocation",
// CHECK:     "Name": "HOST_WIDE_INT",
// CHECK:     "DefinitionLocation": "{{.*}}/Tests/wide-int.cc:14:9",
// CHECK:     "InvocationLocation": "{{.*}}/Tests/wide-int.cc:33:26",
// CHECK:     "ASTKind": "TypeLoc",
// CHECK:     "TypeSignature": "",
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
// CHECK:     "IsHygienic": false,
// CHECK:     "IsICERepresentableByInt32": false,
// CHECK:     "IsDefinitionLocationValid": true,
// CHECK:     "IsInvocationLocationValid": true,
// CHECK:     "IsObjectLike": true,
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
// CHECK:     "IsInvokedWhereConstantExpressionRequired": false,
// CHECK:     "IsAnyArgumentExpandedWhereModifiableValueRequired": false,
// CHECK:     "IsAnyArgumentExpandedWhereAddressableValueRequired": false,
// CHECK:     "IsAnyArgumentConditionallyEvaluated": false,
// CHECK:     "IsAnyArgumentNeverExpanded": false,
// CHECK:     "IsAnyArgumentNotAnExpression": false
// CHECK:   },
// CHECK:   {
// CHECK:     "Kind": "Invocation",
// CHECK:     "Name": "HOST_BITS_PER_WIDE_INT",
// CHECK:     "DefinitionLocation": "{{.*}}/Tests/wide-int.cc:4:9",
// CHECK:     "InvocationLocation": "{{.*}}/Tests/wide-int.cc:20:28",
// CHECK:     "ASTKind": "Expr",
// CHECK:     "TypeSignature": "int HOST_BITS_PER_WIDE_INT",
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
// CHECK:     "IsInvokedWhereConstantExpressionRequired": true,
// CHECK:     "IsAnyArgumentExpandedWhereModifiableValueRequired": false,
// CHECK:     "IsAnyArgumentExpandedWhereAddressableValueRequired": false,
// CHECK:     "IsAnyArgumentConditionallyEvaluated": false,
// CHECK:     "IsAnyArgumentNeverExpanded": false,
// CHECK:     "IsAnyArgumentNotAnExpression": false
// CHECK:   },
// CHECK:   {
// CHECK:     "Kind": "Invocation",
// CHECK:     "Name": "HOST_BITS_PER_WIDE_INT",
// CHECK:     "DefinitionLocation": "{{.*}}/Tests/wide-int.cc:4:9",
// CHECK:     "InvocationLocation": "{{.*}}/Tests/wide-int.cc:20:58",
// CHECK:     "ASTKind": "Expr",
// CHECK:     "TypeSignature": "int HOST_BITS_PER_WIDE_INT",
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
// CHECK:     "IsInvokedWhereConstantExpressionRequired": true,
// CHECK:     "IsAnyArgumentExpandedWhereModifiableValueRequired": false,
// CHECK:     "IsAnyArgumentExpandedWhereAddressableValueRequired": false,
// CHECK:     "IsAnyArgumentConditionallyEvaluated": false,
// CHECK:     "IsAnyArgumentNeverExpanded": false,
// CHECK:     "IsAnyArgumentNotAnExpression": false
// CHECK:   },
// CHECK:   {
// CHECK:     "Kind": "Invocation",
// CHECK:     "Name": "WIDE_INT_MAX_PRECISION",
// CHECK:     "DefinitionLocation": "{{.*}}/Tests/wide-int.cc:8:9",
// CHECK:     "InvocationLocation": "{{.*}}/Tests/wide-int.cc:12:24",
// CHECK:     "ASTKind": "",
// CHECK:     "TypeSignature": "",
// CHECK:     "InvocationDepth": 0,
// CHECK:     "NumASTRoots": 0,
// CHECK:     "NumArguments": 0,
// CHECK:     "HasStringification": false,
// CHECK:     "HasTokenPasting": false,
// CHECK:     "HasAlignedArguments": false,
// CHECK:     "HasSameNameAsOtherDeclaration": false,
// CHECK:     "IsExpansionControlFlowStmt": false,
// CHECK:     "DoesBodyReferenceMacroDefinedAfterMacro": false,
// CHECK:     "DoesBodyReferenceDeclDeclaredAfterMacro": false,
// CHECK:     "DoesBodyContainDeclRefExpr": false,
// CHECK:     "DoesSubexpressionExpandedFromBodyHaveLocalType": false,
// CHECK:     "DoesSubexpressionExpandedFromBodyHaveTypeDefinedAfterMacro": false,
// CHECK:     "DoesAnyArgumentHaveSideEffects": false,
// CHECK:     "DoesAnyArgumentContainDeclRefExpr": false,
// CHECK:     "IsHygienic": false,
// CHECK:     "IsICERepresentableByInt32": false,
// CHECK:     "IsDefinitionLocationValid": true,
// CHECK:     "IsInvocationLocationValid": true,
// CHECK:     "IsObjectLike": true,
// CHECK:     "IsInvokedInMacroArgument": true,
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
// CHECK:     "IsInvokedWhereConstantExpressionRequired": false,
// CHECK:     "IsAnyArgumentExpandedWhereModifiableValueRequired": false,
// CHECK:     "IsAnyArgumentExpandedWhereAddressableValueRequired": false,
// CHECK:     "IsAnyArgumentConditionallyEvaluated": false,
// CHECK:     "IsAnyArgumentNeverExpanded": false,
// CHECK:     "IsAnyArgumentNotAnExpression": false
// CHECK:   }
// CHECK: ]