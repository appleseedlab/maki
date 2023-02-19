from macros import Macro, PreprocessorData


def ie_def(m: Macro, pd: PreprocessorData) -> bool:
    is_ = pd.mm[m]
    # We only analyze top-level non-argument invocations
    assert all([i.IsTopLevelNonArgument for i in is_])
    # We must have semantic data for all invocations
    if not all([i.HasSemanticData for i in is_]):
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
                i.HasSemanticData,

                # Can be turn into an enum or variable
                i.IsObjectLike,
                i.CanBeTurnedIntoEnumOrVariable,

                # Argument-altering
                not i.MustAlterArgumentsOrReturnTypeToTransform,

                # Declaration-altering
                i.DefinitionLocationFilename not in pd.local_includes,
                i.Name not in pd.inspected_macro_names,
                not i.IsNamePresentInCPPConditional,
                not i.MustAlterDeclarationsToTransform,

                # Call-site-context-altering
                not i.MustAlterCallSiteToTransform,

                # Thunkizing
                not i.MustCreateThunksToTransform,

                # Metaprogramming
                not i.MustUseMetaprogrammingToTransform
            ])
            for i in is_
        ])) or
        (m.IsFunctionLike and all([
            all([
                # Valid for analysis
                i.HasSemanticData,

                # Can be turned into a function
                i.IsFunctionLike,
                i.CanBeTurnedIntoFunction,

                # Argument-altering
                not i.MustAlterArgumentsOrReturnTypeToTransform,

                # Declaration-altering
                i.DefinitionLocationFilename not in pd.local_includes,
                i.Name not in pd.inspected_macro_names,
                not i.IsNamePresentInCPPConditional,
                not i.MustAlterDeclarationsToTransform,

                # Call-site-context-altering
                not i.MustAlterCallSiteToTransform,

                # Thunkizing
                not i.MustCreateThunksToTransform,

                # Metaprogramming
                not i.MustUseMetaprogrammingToTransform
            ])
            for i in is_
        ])))
