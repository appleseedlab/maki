#!/usr/bin/python3

from argparse import ArgumentParser
from csv import DictReader
from datetime import timedelta

import numpy as np


def fivenum(data):
    """Five-number summary."""
    return np.percentile(data, [0, 25, 50, 75, 100], method="midpoint")


def parse_timedelta(s: str) -> timedelta:
    days, hours, minutes, seconds, microseconds = [
        int(x) for x in s.split(':')]
    return timedelta(days=days, hours=hours, minutes=minutes, seconds=seconds, microseconds=microseconds)


def main():
    ap = ArgumentParser()
    ap.add_argument('invocation_analysis_times_csv')
    ap.add_argument('definition_analysis_times_csv')
    args = ap.parse_args()

    with open(args.invocation_analysis_times_csv) as macro_analyzer_times, open(args.definition_analysis_times_csv) as property_analyzer_times:
        macro_time_reader = DictReader(macro_analyzer_times)
        property_time_reader = DictReader(property_analyzer_times)

        program_timedeltas = {
            row['Program']: parse_timedelta(row['Time'])
            for row in macro_time_reader
        }

        for row in property_time_reader:
            program_timedeltas[row['Program']] += parse_timedelta(row['Time'])

        tds = list(program_timedeltas.values())

        secs = [td.total_seconds() for td in tds]
        secs.sort()
        print('five point summary:')
        for q in fivenum(secs):
            print(timedelta(seconds=q))

        print()
        print(f'total time: {timedelta(seconds=sum(secs))}')


if __name__ == '__main__':
    main()
