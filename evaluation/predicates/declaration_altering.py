from macros import Invocation, PreprocessorData


def da_invocation(i: Invocation, pd: PreprocessorData) -> bool:
    return i.HasValidSemanticData and all([
        # Valid for analysis
        i.HasValidSemanticData,
        not i.IsAnyArgumentNeverExpanded,

        # Can be transformed to a function or variable
        i.CanBeTurnedIntoAFunctionOrVariable,

        # Alignment (also used for call-site-context-altering)
        i.IsAligned,

        # Argument-altering
        # i.UsesArgumentsFromCallerScope,

        # Declaration-altering
        any([i.DefinitionLocationFilename in pd.local_includes,
             i.Name in pd.inspected_macro_names,
             i.IsNamePresentInCPPConditional,
             i.ExploitsMacroDeclarationSemantics]),

        # Call-site-context-altering
        # not i.IsExpansionControlFlowStmt,
        # not i.IsAnyArgumentConditionallyEvaluated,

        # Thunkizing
        # not i.DoesAnyArgumentHaveSideEffects,
        # not i.IsAnyArgumentNotAnExpression,
        # not i.IsAnyArgumentTypeVoid,

        # Metaprogramming
        # not i.InvolvesMetaprogramming
    ])
