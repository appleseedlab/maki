from macros import Invocation, PreprocessorData


def mp_invocation(i: Invocation, pd: PreprocessorData) -> bool:
    assert i.IsTopLevelNonArgument
    return i.MustUseMetaprogrammingToTransform
