from macros import Invocation, PreprocessorData


def syntactic(i: Invocation, _pd: PreprocessorData) -> bool:
    return i.SatisfiesASyntacticProperty


def scoping(i: Invocation, _pd: PreprocessorData) -> bool:
    return i.HasSemanticData and i.SatisfiesAScopingRuleProperty


def typing(i: Invocation, _pd: PreprocessorData) -> bool:
    return i.HasSemanticData and i.SatisfiesATypingProperty


def calling_convention(i: Invocation, _pd: PreprocessorData) -> bool:
    return i.HasSemanticData and i.SatisfiesACallingConventionProperty


def language_specific(i: Invocation, _pd: PreprocessorData) -> bool:
    return i.SatisfiesALanguageSpecificProperty


PROPERTY_CATEGORIES = [syntactic, scoping, typing,
                       calling_convention, language_specific]
