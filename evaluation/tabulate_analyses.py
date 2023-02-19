#!/usr/bin/python3

import csv
import json
import os
from dataclasses import astuple, fields
from typing import Tuple

from analysis import Analysis

ANALYSES_DIR = r'analyses/'
DELIM = ','


def load_analysis_file(path: str) -> Analysis:
    with open(path) as fp:
        return Analysis(**json.load(fp))


def pcnt(n, d):
    return round(n / d * 100, 2) if d else 'N/A'


def write_table(ofp, macro_type: str) -> None:
    w = csv.writer(ofp, delimiter=DELIM, quoting=csv.QUOTE_MINIMAL)

    headers = None
    totals = None

    for path in sorted(os.listdir(ANALYSES_DIR)):
        name = os.path.basename(path)
        if '-' in name:
            name = name[:name.find('-')]
        if name.endswith('.json'):
            name = name[:name.find('.json')]

        fullpath = os.path.join(ANALYSES_DIR, path)
        a = load_analysis_file(fullpath)

        row_data = [stat[macro_type] for stat in astuple(a)]

        if headers is None:
            # Write the header row first if we haven't already written it
            headers = ['program_name', *[f.name for f in fields(a)]]
            w.writerow(headers)

        if totals is None:
            # Initialize totals for each program if we haven't already
            totals = row_data[:]
        else:
            totals = [acc + nxt for acc, nxt in zip(totals, row_data)]

        w.writerow([name, *row_data])

    w.writerow(['total', *totals])


def main():
    # TODO: Add to analysis number of unique unaligned invocations at valid src locs

    with open('olm_table.csv', 'w', newline='') as ofp:
        write_table(ofp, 'olms')

    with open('flm_table.csv', 'w', newline='') as ofp:
        write_table(ofp, 'flms')

    with open('total_table.csv', 'w', newline='') as ofp:
        write_table(ofp, 'total')


if __name__ == '__main__':
    main()
