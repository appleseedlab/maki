#!/usr/bin/python3

from argparse import ArgumentParser
from csv import DictReader
from operator import itemgetter


def extract_percentage(s: str) -> float:
    return float(s[(s.index('(') + 1): s.index('%')])


def main():
    ap = ArgumentParser()
    ap.add_argument('data_file')
    args = ap.parse_args()

    with open(args.data_file) as fp:
        reader = DictReader(fp)
        program_alignment_adaptations = []
        for row in reader:
            program_alignment_adaptations.append(
                (row['program'],
                 round(sum(map(extract_percentage, [
                     row['src_definitions_with_only_stmt_invocations'],
                     row['src_definitions_with_only_expr_invocations'],
                     row['src_definitions_with_only_decl_invocations'],
                     row['src_definitions_with_only_type_loc_invocations'],
                 ])), 2),
                 round(sum(map(extract_percentage, [
                     row['interface_equivalent_src_definitions'],
                     row['src_definitions_with_only_argument_altering_invocations'],
                     row['src_definitions_with_only_declaration_altering_invocations'],
                     row['macros_defined_at_valid_src_locs_with_multiple_easy_transformations'],
                 ])), 2))
            )

        _, total_aligned, total_interface_equivalent = program_alignment_adaptations.pop()
        program_alignment_adaptations.sort(key=itemgetter(1), reverse=True)
        program_alignment_adaptations.append(
            ('Total', total_aligned, total_interface_equivalent))

        print('Program,Percentage of aligned definitions,Percentage of interface-equivalent definitions')
        for p, a, b in program_alignment_adaptations:
            print(f'{p},{a},{b}')


if __name__ == '__main__':
    main()
