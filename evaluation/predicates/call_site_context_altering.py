from macros import Invocation, PreprocessorData


def csca_invocation(i: Invocation, pd: PreprocessorData) -> bool:
    return (i.HasValidSemanticData and
            # If not aligned, then call-site-context-altering
            (not i.IsAligned or
             all([
                 # Valid for analysis
                 i.HasValidSemanticData,
                 not i.IsAnyArgumentNeverExpanded,

                 # Can be transformed to a function or variable
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
                 any([i.IsExpansionControlFlowStmt,
                      i.IsAnyArgumentConditionallyEvaluated]),

                 # Thunkizing
                 # not i.DoesAnyArgumentHaveSideEffects,
                 # not i.IsAnyArgumentNotAnExpression,
                 # not i.IsAnyArgumentTypeVoid,

                 # Metaprogramming
                 # not i.InvolvesMetaprogramming
             ])))
