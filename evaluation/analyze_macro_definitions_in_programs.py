#!/usr/bin/python3


import argparse
import os
import sys
from datetime import datetime
from subprocess import run

DEFINITION_ANALYSES_DIR = r'macro_definition_analyses/'
INVOCATION_ANALYSES_DIR = r'macro_invocation_analyses/'


def main():

    ap = argparse.ArgumentParser()
    ap.add_argument('macro_definition_analysis_time_output_file', type=str)
    args = ap.parse_args()

    os.makedirs(DEFINITION_ANALYSES_DIR, exist_ok=True)

    dir_names = list(sorted(os.listdir(INVOCATION_ANALYSES_DIR)))

    result_files = [
        os.path.join(INVOCATION_ANALYSES_DIR, dir_name, 'all_results.cpp2c')
        for dir_name in dir_names
    ]

    with open(args.macro_definition_analysis_time_output_file, 'w') as ofp:
        print('Program,Time', file=ofp)
        for dir_name, result_fn in zip(dir_names, result_files):
            if not os.path.isfile(result_fn):
                print(f'warning: skipping {result_fn} because it does not exist')
                continue
            output_file = os.path.join(DEFINITION_ANALYSES_DIR, f'{dir_name}.json')
            # TODO: Add an option to overwrite analyses if they already exist
            if not os.path.isfile(output_file):
                cmd = f'python3 analyze_macro_definitions_in_program.py "{result_fn}" -o="{output_file}"'
                print(cmd)
                t0 = datetime.now()
                run(cmd, shell=True)
                t1 = datetime.now()
                delta = t1 - t0
                delta_formatted = f'{delta.days}:{delta.seconds // 3600}:{delta.seconds // 60}:{delta.seconds}:{delta.microseconds}'
                print(f'{dir_name},{delta_formatted}', file=ofp)
                sys.stdout.flush()


if __name__ == '__main__':
    main()
