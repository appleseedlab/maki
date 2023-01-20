from macros import Invocation, PreprocessorData


def mp_invocation(i: Invocation, pd: PreprocessorData) -> bool:
    return i.InvolvesMetaprogramming
