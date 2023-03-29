#!/usr/bin/python3

from argparse import ArgumentParser
from csv import DictReader
from operator import itemgetter


def main():
    ap = ArgumentParser()
    ap.add_argument('data_file')
    args = ap.parse_args()

    with open(args.data_file) as fp:
        reader = DictReader(fp)
        program_improvement = [
            (row['program'],
             float(row['x_more_easy_to_transform_macros_we_find_than_mennie']))
            for row in reader
        ]

        total_improvement = program_improvement.pop()[1]

        program_improvement.sort(key=itemgetter(1), reverse=True)
        program_improvement.append(('Total', total_improvement))

        print('Program,How times more easily portable macros Maki finds over prior work')
        for program, improvement in program_improvement:
            print(f'{program},{improvement}')


if __name__ == '__main__':
    main()
