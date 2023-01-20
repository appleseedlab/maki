from macros import Invocation, PreprocessorData


def aa_invocation(i: Invocation, pd: PreprocessorData) -> bool:
    return i.HasValidSemanticData and all([
        # Valid for analysis
        i.HasValidSemanticData,
        not i.IsAnyArgumentNeverExpanded,

        # Can be turned into a function or variable
        i.CanBeTurnedIntoAFunctionOrVariable,

        # Alignment (also used for call-site-context-altering)
        i.IsAligned,

        # Argument-altering
        i.ExploitsDynamicScoping,

        # # Declaration-altering
        # i.DefinitionLocationFilename not in pd.local_includes,
        # i.Name not in pd.inspected_macro_names,
        # not i.IsNamePresentInCPPConditional,
        # not i.ExploitsMacroDeclarationSemantics,

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
