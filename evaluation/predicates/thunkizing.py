from macros import Invocation, PreprocessorData


def thunkizing_invocation(i: Invocation, pd: PreprocessorData) -> bool:
    assert i.IsTopLevelNonArgument
    return (i.HasSemanticData and
            # Must be able to turn into a function
            i.CanBeTurnedIntoFunction and
            # Object-like to thunk
            (i.IsObjectLike and i.IsExpansionTypeVoid) or
            # Turn arguments into thunks
            (i.IsFunctionLike and i.MustCreateThunksToTransform))
