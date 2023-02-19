from dataclasses import dataclass
from typing import Callable

from macros import Invocation, Macro, PreprocessorData


@dataclass
class MacroStat:
    olms: int
    flms: int
    total: int


def definition_stat(
        pd: PreprocessorData,
        p: Callable[[Macro, PreprocessorData], bool]) -> MacroStat:
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

    return MacroStat(olms, flms, total)


def invocation_stat(
        pd: PreprocessorData,
        p: Callable[[Invocation, PreprocessorData], bool]) -> MacroStat:
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

    return MacroStat(olms, flms, total)


@dataclass
class Analysis:
    defined_macros: MacroStat
    macros_defined_at_valid_src_locs: MacroStat
    macros_defined_at_valid_src_locs_with_only_top_level_non_argument_invocations: MacroStat

    src_invocations_at_unique_locations: MacroStat
    src_invocations_at_unique_valid_locations: MacroStat
    src_invocations_at_unique_invalid_locations: MacroStat

    nested_argument_src_invocations: MacroStat
    nested_non_argument_src_invocations: MacroStat
    top_level_argument_src_invocations: MacroStat
    top_level_non_argument_src_invocations: MacroStat

    top_level_non_argument_src_invocations_with_semantic_data: MacroStat

    interface_equivalent_src_definitions: MacroStat
    top_level_non_argument_interface_equivalent_src_invocations: MacroStat

    mennie_definitions: MacroStat
    mennie_invocations: MacroStat

    src_definitions_with_only_argument_altering_invocations: MacroStat
    src_definitions_with_any_argument_altering_invocations: MacroStat
    top_level_non_argument_src_invocations_that_are_only_argument_altering: MacroStat
    top_level_non_argument_src_invocations_that_are_at_least_argument_altering: MacroStat

    src_definitions_with_only_declaration_altering_invocations: MacroStat
    src_definitions_with_any_declaration_altering_invocations: MacroStat
    top_level_non_argument_src_invocations_that_are_only_declaration_altering: MacroStat
    top_level_non_argument_src_invocations_that_are_at_least_declaration_altering: MacroStat

    src_definitions_with_only_call_site_context_altering_invocations: MacroStat
    src_definitions_with_any_call_site_context_altering_invocations: MacroStat
    top_level_non_argument_src_invocations_that_are_only_call_site_context_altering: MacroStat
    top_level_non_argument_src_invocations_that_are_at_least_call_site_context_altering: MacroStat

    src_definitions_with_only_thunkizing_invocations: MacroStat
    src_definitions_with_any_thunkizing_invocations: MacroStat
    top_level_non_argument_src_invocations_that_are_only_thunkizing: MacroStat
    top_level_non_argument_src_invocations_that_are_at_least_thunkizing: MacroStat

    src_definitions_with_only_metaprogramming_invocations: MacroStat
    src_definitions_with_any_metaprogramming_invocations: MacroStat
    top_level_non_argument_src_invocations_that_are_only_metaprogramming: MacroStat
    top_level_non_argument_src_invocations_that_are_at_least_metaprogramming: MacroStat

    src_definitions_that_are_easy_to_transform: MacroStat
    top_level_non_argument_src_invocations_that_are_easy_to_transform: MacroStat

    src_definitions_that_are_hard_to_transform: MacroStat
    top_level_non_argument_src_invocations_that_are_hard_to_transform: MacroStat

    avg_top_level_non_argument_src_invocations_per_easy_to_transform_definition: MacroStat
    avg_top_level_non_argument_src_invocations_per_hard_to_transform_definition: MacroStat

    src_definitions_with_only_decl_invocations: MacroStat
    top_level_non_argument_decl_invocations: MacroStat

    src_definitions_with_only_stmt_invocations: MacroStat
    top_level_non_argument_stmt_invocations: MacroStat

    src_definitions_with_only_expr_invocations: MacroStat
    top_level_non_argument_expr_invocations: MacroStat

    src_definitions_with_only_type_loc_invocations: MacroStat
    top_level_non_argument_type_loc_invocations: MacroStat

    src_definitions_with_mixed_syntax_invocations: MacroStat

    src_definitions_with_invocations_that_only_satisfy_syntactic_properties: MacroStat
    src_definitions_with_any_invocation_that_satisfies_syntactic_properties: MacroStat
    top_level_non_argument_src_invocations_that_only_satisfy_syntactic_properties: MacroStat
    top_level_non_argument_src_invocations_that_at_least_satisfy_syntactic_properties: MacroStat

    src_definitions_with_invocations_that_only_satisfy_scoping_properties: MacroStat
    src_definitions_with_any_invocation_that_satisfies_scoping_properties: MacroStat
    top_level_non_argument_src_invocations_that_only_satisfy_scoping_properties: MacroStat
    top_level_non_argument_src_invocations_that_at_least_satisfy_scoping_properties: MacroStat

    src_definitions_with_invocations_that_only_satisfy_typing_properties: MacroStat
    src_definitions_with_any_invocation_that_satisfies_typing_properties: MacroStat
    top_level_non_argument_src_invocations_that_only_satisfy_typing_properties: MacroStat
    top_level_non_argument_src_invocations_that_at_least_satisfy_typing_properties: MacroStat

    src_definitions_with_invocations_that_only_satisfy_calling_convention_properties: MacroStat
    src_definitions_with_any_invocation_that_satisfies_calling_convention_properties: MacroStat
    top_level_non_argument_src_invocations_that_only_satisfy_calling_convention_properties: MacroStat
    top_level_non_argument_src_invocations_that_at_least_satisfy_calling_convention_properties: MacroStat

    src_definitions_with_invocations_that_only_satisfy_language_specific_properties: MacroStat
    src_definitions_with_any_invocation_that_satisfies_language_specific_properties: MacroStat
    top_level_non_argument_src_invocations_that_only_satisfy_language_specific_properties: MacroStat
    top_level_non_argument_src_invocations_that_at_least_satisfy_language_specific_properties: MacroStat

    src_definitions_with_invocations_that_satisfy_no_properties: MacroStat
    src_definitions_with_any_invocation_that_satisfies_no_properties: MacroStat
    top_level_non_argument_src_invocations_that_satisfy_no_properties: MacroStat
