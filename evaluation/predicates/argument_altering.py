from macros import Invocation, PreprocessorData


def aa_invocation(i: Invocation, pd: PreprocessorData) -> bool:
    assert i.IsTopLevelNonArgument
    return i.HasSemanticData and all([
        # Can be turned into a function or variable
        i.CanBeTurnedIntoAFunctionOrVariable,
        # Argument-altering
        i.MustAlterArgumentsOrReturnTypeToTransform,
    ])
