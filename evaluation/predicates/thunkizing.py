from macros import Invocation, PreprocessorData


def thunkizing_invocation(i: Invocation, pd: PreprocessorData) -> bool:
    return (i.HasValidSemanticData and
            ((i.IsObjectLike and i.IsExpansionTypeVoid) or
             i.IsFunctionLike and all([
                 # Valid for analysis
                 i.HasValidSemanticData,
                 not i.IsAnyArgumentNeverExpanded,

                 # Can be replaced with a function
                 i.IsFunctionLike,
                 i.CanBeTurnedIntoFunction,

                 # Alignment (also used for call-site-context-altering)
                 i.IsAligned,

                 # Argument-altering
                 # i.ExploitsDynamicScoping,

                 # # Declaration-altering
                 # i.DefinitionLocationFilename not in pd.local_includes,
                 # i.Name not in pd.inspected_macro_names,
                 # not i.IsNamePresentInCPPConditional,
                 # not i.ExploitsMacroDeclarationSemantics,

                 # Call-site-context-altering
                 # not i.IsExpansionControlFlowStmt,
                 # not i.IsAnyArgumentConditionallyEvaluated,

                 # Thunkizing
                 any([i.DoesAnyArgumentHaveSideEffects,
                      i.IsAnyArgumentNotAnExpression,
                      i.IsAnyArgumentTypeVoid]),

                 # Metaprogramming
                 # not i.InvolvesMetaprogramming
             ])))
