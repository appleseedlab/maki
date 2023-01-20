from dataclasses import dataclass
from typing import Callable

from macros import Invocation, Macro, PreprocessorData


@dataclass
class MacroOFStat:
    olms: int
    flms: int
    total: int


def definition_stat(
        pd: PreprocessorData,
        p: Callable[[Macro, PreprocessorData], bool]) -> MacroOFStat:
    '''
    Returns which macro definitions in the PreprocessorData object
    satisfy the given predicate.
    '''
    olms, flms, total = 0, 0, 0
    for m in pd.mm.keys():
        if m.IsObjectLike and p(m, pd):
            olms += 1
            total += 1
        elif m.IsFunctionLike and p(m, pd):
            flms += 1
            total += 1

    return MacroOFStat(olms, flms, total)


def invocation_stat(
        pd: PreprocessorData,
        p: Callable[[Invocation, PreprocessorData], bool]) -> MacroOFStat:
    '''
    Returns which macro invocations in the PreprocessorData object
    satisfy the given predicate.
    '''
    olms, flms, total = 0, 0, 0
    for m, is_ in pd.mm.items():
        if m.IsObjectLike:
            for i in is_:
                if p(i, pd):
                    olms += 1
                    total += 1
        elif m.IsFunctionLike:
            for i in is_:
                if p(i, pd):
                    flms += 1
                    total += 1

    return MacroOFStat(olms, flms, total)


@dataclass
class Analysis:
    defined_macros: MacroOFStat
    macros_defined_at_valid_src_locs: MacroOFStat

    src_invocations_at_unique_locations: MacroOFStat
    src_invocations_at_unique_valid_locations: MacroOFStat
    src_invocations_at_unique_invalid_locations: MacroOFStat

    nested_argument_src_invocations: MacroOFStat
    nested_non_argument_src_invocations: MacroOFStat
    toplevel_argument_src_invocations: MacroOFStat
    toplevel_non_argument_src_invocations: MacroOFStat

    aligned_src_invocations: MacroOFStat

    interface_equivalent_src_definitions: MacroOFStat
    interface_equivalent_src_invocations: MacroOFStat

    src_definitions_with_only_argument_altering_invocations: MacroOFStat
    src_definitions_with_any_argument_altering_invocations: MacroOFStat
    toplevel_non_argument_src_invocations_that_are_only_argument_altering: MacroOFStat
    toplevel_non_argument_src_invocations_that_are_at_least_argument_altering: MacroOFStat

    src_definitions_with_only_declaration_altering_invocations: MacroOFStat
    src_definitions_with_any_declaration_altering_invocations: MacroOFStat
    toplevel_non_argument_src_invocations_that_are_only_declaration_altering: MacroOFStat
    toplevel_non_argument_src_invocations_that_are_at_least_declaration_altering: MacroOFStat

    src_definitions_with_only_call_site_context_altering_invocations: MacroOFStat
    src_definitions_with_any_call_site_context_altering_invocations: MacroOFStat
    toplevel_non_argument_src_invocations_that_are_only_call_site_context_altering: MacroOFStat
    toplevel_non_argument_src_invocations_that_are_at_least_call_site_context_altering: MacroOFStat

    src_definitions_with_only_thunkizing_invocations: MacroOFStat
    src_definitions_with_any_thunkizing_invocations: MacroOFStat
    toplevel_non_argument_src_invocations_that_are_only_thunkizing: MacroOFStat
    toplevel_non_argument_src_invocations_that_are_at_least_thunkizing: MacroOFStat

    src_definitions_with_only_metaprogramming_invocations: MacroOFStat
    src_definitions_with_any_metaprogramming_invocations: MacroOFStat
    toplevel_non_argument_src_invocations_that_are_only_metaprogramming: MacroOFStat
    toplevel_non_argument_src_invocations_that_are_at_least_metaprogramming: MacroOFStat
