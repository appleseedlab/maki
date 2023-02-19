from macros import Invocation, PreprocessorData


def csca_invocation(i: Invocation, pd: PreprocessorData) -> bool:
    assert i.IsTopLevelNonArgument
    # If not aligned, then call-site-context-altering
    return (not i.IsAligned or
            # Must have semantic data
            (i.HasSemanticData and all([
                # Can be transformed to a function or variable
                i.CanBeTurnedIntoAFunctionOrVariable,
                # Call-site altering
                i.MustAlterCallSiteToTransform
            ])))
