#!/usr/bin/python3

from argparse import ArgumentParser
from csv import DictReader


def extract_raw_data(s: str) -> float:
    return int(s[: s.index('(')])


def main():
    ap = ArgumentParser()
    ap.add_argument('data_file')
    args = ap.parse_args()

    with open(args.data_file) as fp:
        reader = DictReader(fp)
        rows = []
        program_to_macros_defined_at_valid_src_locs = {}
        for row in reader:
            rows.append(
                (row['program'],
                 *map(extract_raw_data, [
                     row[field]
                     for field in
                     [
                         'interface_equivalent_src_definitions',
                         'src_definitions_with_only_argument_altering_invocations',
                         'src_definitions_with_only_declaration_altering_invocations',
                         'macros_defined_at_valid_src_locs_with_multiple_easy_transformations',
                         'src_definitions_with_only_call_site_context_altering_invocations',
                         'src_definitions_with_only_thunkizing_invocations',
                         'src_definitions_with_only_metaprogramming_invocations',
                         'macros_defined_at_valid_src_locs_with_nested_invocations',
                         'macros_defined_at_valid_src_locs_with_multiple_hard_transformations',
                     ]])
                 ))
            program_to_macros_defined_at_valid_src_locs[row['program']] = \
                row['macros_defined_at_valid_src_locs']

        total_row = rows.pop()
        rows.sort(key=(lambda row:
                       (sum(row[1:5])
                        / int(program_to_macros_defined_at_valid_src_locs[row[0]]))),
                  reverse=True)
        rows.append(('Total', *total_row[1:]))

        print('Program,Definition-adapting,Calling-convention-adapting,Scope-adapting,Multiple interface-equivalent adaptations,Call-site-context-altering,Thunkizing,Metaprogramming,Nested invocations,Multiple non-interface-equivalent adaptations')
        for row in rows:
            print(','.join(map(str, row)))


if __name__ == '__main__':
    main()
