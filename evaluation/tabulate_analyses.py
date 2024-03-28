#!/usr/bin/python3

import csv
import json
import os
from math import inf

DEFINITION_ANALYSES_DIR = r'macro_definition_analyses/'
DELIM = ','

DENOMINATOR_FIELDS = {
    'macros_defined_at_valid_src_locs',
    'src_invocations_at_unique_valid_locations'
}

FIELD_NAMES = ['program', 'defined_macros', 'macros_defined_at_valid_src_locs', 'macros_defined_at_valid_src_locs_with_only_top_level_non_argument_invocations', 'macros_defined_at_valid_src_locs_with_nested_invocations',  'src_invocations_at_unique_locations', 'src_invocations_at_unique_valid_locations', 'src_invocations_at_unique_invalid_locations', 'nested_argument_src_invocations', 'nested_non_argument_src_invocations', 'top_level_argument_src_invocations', 'top_level_non_argument_src_invocations', 'top_level_non_argument_src_invocations_with_semantic_data', 'interface_equivalent_src_definitions', 'top_level_non_argument_interface_equivalent_src_invocations', 'mennie_definitions', 'mennie_invocations', 'x_more_easy_to_transform_macros_we_find_than_mennie', 'src_definitions_with_only_argument_altering_invocations', 'src_definitions_with_any_argument_altering_invocations', 'top_level_non_argument_src_invocations_that_are_only_argument_altering', 'top_level_non_argument_src_invocations_that_are_at_least_argument_altering', 'src_definitions_with_only_declaration_altering_invocations', 'src_definitions_with_any_declaration_altering_invocations', 'top_level_non_argument_src_invocations_that_are_only_declaration_altering', 'top_level_non_argument_src_invocations_that_are_at_least_declaration_altering', 'src_definitions_with_only_call_site_context_altering_invocations', 'src_definitions_with_any_call_site_context_altering_invocations', 'top_level_non_argument_src_invocations_that_are_only_call_site_context_altering', 'top_level_non_argument_src_invocations_that_are_at_least_call_site_context_altering', 'src_definitions_with_only_thunkizing_invocations', 'src_definitions_with_any_thunkizing_invocations', 'top_level_non_argument_src_invocations_that_are_only_thunkizing', 'top_level_non_argument_src_invocations_that_are_at_least_thunkizing', 'src_definitions_with_only_metaprogramming_invocations', 'src_definitions_with_any_metaprogramming_invocations', 'top_level_non_argument_src_invocations_that_are_only_metaprogramming', 'top_level_non_argument_src_invocations_that_are_at_least_metaprogramming', 'src_definitions_that_are_easy_to_transform', 'top_level_non_argument_src_invocations_that_are_easy_to_transform', 'macros_defined_at_valid_src_locs_with_multiple_easy_transformations', 'src_definitions_that_are_hard_to_transform', 'top_level_non_argument_src_invocations_that_are_hard_to_transform', 'macros_defined_at_valid_src_locs_with_multiple_hard_transformations', 'avg_top_level_non_argument_src_invocations_per_easy_to_transform_definition',
               'avg_top_level_non_argument_src_invocations_per_hard_to_transform_definition', 'src_definitions_with_only_decl_invocations', 'top_level_non_argument_decl_invocations', 'src_definitions_with_only_stmt_invocations', 'top_level_non_argument_stmt_invocations', 'src_definitions_with_only_expr_invocations', 'top_level_non_argument_expr_invocations', 'src_definitions_with_only_type_loc_invocations', 'top_level_non_argument_type_loc_invocations', 'top_level_non_argument_unaligned_invocations', 'src_definitions_with_mixed_syntax_invocations', 'src_definitions_with_invocations_that_only_satisfy_syntactic_properties', 'src_definitions_with_any_invocation_that_satisfies_syntactic_properties', 'top_level_non_argument_src_invocations_that_only_satisfy_syntactic_properties', 'top_level_non_argument_src_invocations_that_at_least_satisfy_syntactic_properties', 'src_definitions_with_invocations_that_only_satisfy_scoping_properties', 'src_definitions_with_any_invocation_that_satisfies_scoping_properties', 'top_level_non_argument_src_invocations_that_only_satisfy_scoping_properties', 'top_level_non_argument_src_invocations_that_at_least_satisfy_scoping_properties', 'src_definitions_with_invocations_that_only_satisfy_typing_properties', 'src_definitions_with_any_invocation_that_satisfies_typing_properties', 'top_level_non_argument_src_invocations_that_only_satisfy_typing_properties', 'top_level_non_argument_src_invocations_that_at_least_satisfy_typing_properties', 'src_definitions_with_invocations_that_only_satisfy_calling_convention_properties', 'src_definitions_with_any_invocation_that_satisfies_calling_convention_properties', 'top_level_non_argument_src_invocations_that_only_satisfy_calling_convention_properties', 'top_level_non_argument_src_invocations_that_at_least_satisfy_calling_convention_properties', 'src_definitions_with_invocations_that_only_satisfy_language_specific_properties', 'src_definitions_with_any_invocation_that_satisfies_language_specific_properties', 'top_level_non_argument_src_invocations_that_only_satisfy_language_specific_properties', 'top_level_non_argument_src_invocations_that_at_least_satisfy_language_specific_properties', 'src_definitions_with_invocations_that_satisfy_no_properties', 'src_definitions_with_any_invocation_that_satisfies_no_properties', 'top_level_non_argument_src_invocations_that_satisfy_no_properties']


def load_analysis_file(path: str) -> dict:
    with open(path) as fp:
        return json.load(fp)


def pcnt(n: float, d: float) -> float | str:
    return round(n / d * 100, 2) if d else 'N/A'


def stat_percent(row_data: dict, numerator_field: str) -> float:
    return pcnt(row_data[numerator_field],
                (row_data[
                    'macros_defined_at_valid_src_locs'
                    if 'defin' in numerator_field
                    else 'src_invocations_at_unique_valid_locations']))


def write_table(ofp, macro_type: str) -> None:
    w = csv.DictWriter(ofp, FIELD_NAMES, delimiter=DELIM,
                       quoting=csv.QUOTE_MINIMAL)
    w.writeheader()

    totals = None

    for path in sorted(os.listdir(DEFINITION_ANALYSES_DIR)):
        name = os.path.basename(path)
        if '-' in name:
            name = name[:name.find('-')]
        if name.endswith('.json'):
            name = name[:name.find('.json')]
        if name == 'ncsa':
            name = 'mosaic'

        fullpath = os.path.join(DEFINITION_ANALYSES_DIR, path)
        a = load_analysis_file(fullpath)
        row_data = {key: a[key][macro_type] for key in a.keys()}
        row_data['program'] = name 

        # NOTE: We currently count source definitions of unknown difficulty
        # as hard to transform

        row_data['top_level_non_argument_unaligned_invocations'] = (
            row_data['src_invocations_at_unique_valid_locations']
            - row_data['top_level_non_argument_decl_invocations']
            - row_data['top_level_non_argument_stmt_invocations']
            - row_data['top_level_non_argument_expr_invocations']
            - row_data['top_level_non_argument_type_loc_invocations']
            - row_data['nested_argument_src_invocations']
            - row_data['nested_non_argument_src_invocations']
            - row_data['top_level_argument_src_invocations']
        )

        row_data['macros_defined_at_valid_src_locs_with_nested_invocations'] = (
            row_data['macros_defined_at_valid_src_locs']
            - row_data['macros_defined_at_valid_src_locs_with_only_top_level_non_argument_invocations']
        )

        row_data['macros_defined_at_valid_src_locs_with_multiple_easy_transformations'] = (
            row_data['src_definitions_that_are_easy_to_transform']
            - row_data['interface_equivalent_src_definitions']
            - row_data['src_definitions_with_only_argument_altering_invocations']
            - row_data['src_definitions_with_only_declaration_altering_invocations']
        )

        row_data['macros_defined_at_valid_src_locs_with_multiple_hard_transformations'] = (
            row_data['src_definitions_that_are_hard_to_transform']
            - row_data['src_definitions_with_only_call_site_context_altering_invocations']
            - row_data['src_definitions_with_only_thunkizing_invocations']
            - row_data['src_definitions_with_only_metaprogramming_invocations']
        )

        row_data['x_more_easy_to_transform_macros_we_find_than_mennie'] = (
            round(row_data['src_definitions_that_are_easy_to_transform']
                  / row_data['mennie_definitions'], 2)
            if row_data['mennie_definitions'] > 0 else inf
        )

        if totals is None:
            # Initialize totals for each program if we haven't already
            totals = {key: val for key, val in row_data.items()}
            totals['program'] = 'total'
        else:
            for key in row_data.keys():
                if key != 'program':
                    totals[key] += row_data[key]

        for key in row_data.keys():
            if key != 'program' and key != 'x_more_easy_to_transform_macros_we_find_than_mennie' and key not in DENOMINATOR_FIELDS:
                row_data[key] = f'{row_data[key]} ({stat_percent(row_data, key)}%)'

        w.writerow(row_data)

    # Overall improvement
    totals['x_more_easy_to_transform_macros_we_find_than_mennie'] = (
        round(totals['src_definitions_that_are_easy_to_transform']
              / totals['mennie_definitions'], 2)
        if totals['mennie_definitions'] > 0 else inf
    )

    # Calculate percentages for totals
    for key in totals.keys():
        if key not in ['program', 'x_more_easy_to_transform_macros_we_find_than_mennie', *DENOMINATOR_FIELDS]:
            totals[key] = f'{round(totals[key], 2)} ({stat_percent(totals, key)}%)'

    w.writerow(totals)


def main():
    with open('all_raw_data.csv', 'w', newline='') as ofp:
        write_table(ofp, 'total')


if __name__ == '__main__':
    main()
