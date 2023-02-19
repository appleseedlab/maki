from macros import Invocation, PreprocessorData


def da_invocation(i: Invocation, pd: PreprocessorData) -> bool:
    assert i.IsTopLevelNonArgument
    return i.HasSemanticData and (
        # Option 1: Declaration-altering function or variable
        all([
            # Can be transformed to a function or variable
            i.CanBeTurnedIntoAFunctionOrVariable,
            # Declaration-altering
            any([i.DefinitionLocationFilename in pd.local_includes,
                 i.Name in pd.inspected_macro_names,
                 i.IsNamePresentInCPPConditional,
                 i.MustAlterDeclarationsToTransform]),
        ]) or
        # Option 2: Typedef transformation
        all([
            # Can be transformed into a typedef
            i.IsObjectLike,
            i.CanBeTurnedIntoTypeDef,
        ]))
