from macros import Macro, PreprocessorData


def ie_def(m: Macro, pd: PreprocessorData) -> bool:
    is_ = pd.mm[m]
    if (any([not i.HasValidSemanticData for i in is_])):
        return False
    # The macro must be expanded at least once
    if len(is_) == 0:
        return False
    # All invocations must have the same type signature
    if len(set([i.TypeSignature for i in is_])) != 1:
        return False
    return (
        (m.IsObjectLike and all([
            all([
                # Valid for analysis
                i.HasValidSemanticData,
                # not i.IsAnyArgumentNeverExpanded,

                # Can be turn into a variable
                i.IsObjectLike,
                i.CanBeTurnedIntoVariable or i.CanBeTurnedIntoEnum,

                # Alignment (also used for callsite-context-altering)
                i.IsAligned,

                # Argument-altering
                not i.ExploitsDynamicScoping,

                # Declaration-altering
                i.DefinitionLocationFilename not in pd.local_includes,
                i.Name not in pd.inspected_macro_names,
                not i.IsNamePresentInCPPConditional,
                not i.ExploitsMacroDeclarationSemantics,

                # Call-site-context-altering
                not i.IsExpansionControlFlowStmt,
                # not i.IsAnyArgumentConditionallyEvaluated,

                # Thunkizing
                # not i.DoesAnyArgumentHaveSideEffects,
                # not i.IsAnyArgumentNotAnExpression,
                # not i.IsAnyArgumentTypeVoid,

                # Metaprogramming
                not i.InvolvesMetaprogramming
            ])
            for i in is_
        ])) or
        (m.IsFunctionLike and all([
            all([
                # Valid for analysis
                i.HasValidSemanticData,
                not i.IsAnyArgumentNeverExpanded,

                # Can be turned into a function
                i.IsFunctionLike,
                i.CanBeTurnedIntoFunction,

                # Alignment (also used for callsite-context-altering)
                i.IsAligned,

                # Argument-altering
                not i.ExploitsDynamicScoping,

                # Declaration-altering
                i.DefinitionLocationFilename not in pd.local_includes,
                i.Name not in pd.inspected_macro_names,
                not i.IsNamePresentInCPPConditional,
                not i.ExploitsMacroDeclarationSemantics,

                # Call-site-context-altering
                not i.IsExpansionControlFlowStmt,
                not i.IsAnyArgumentConditionallyEvaluated,

                # Thunkizing
                not i.DoesAnyArgumentHaveSideEffects,
                not i.IsAnyArgumentNotAnExpression,
                not i.IsAnyArgumentTypeVoid,

                # Metaprogramming
                not i.InvolvesMetaprogramming
            ])
            for i in is_
        ])))
