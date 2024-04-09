#!/usr/bin/python3


import os
from datetime import datetime
import sys
from subprocess import run

ANALYSES_DIR = r'analyses/'
RESULTS_DIR = r'results/'


def main():

    os.makedirs(ANALYSES_DIR, exist_ok=True)

    dir_names = list(sorted(os.listdir(RESULTS_DIR)))

    result_files = [
        os.path.join(RESULTS_DIR, dir_name, 'all_results.cpp2c')
        for dir_name in dir_names
    ]

    for dir_name, result_fn in zip(dir_names, result_files):
        if not os.path.isfile(result_fn):
            print(f'warning: skipping {result_fn} because it does not exist')
            continue
        output_file = os.path.join(ANALYSES_DIR, f'{dir_name}.json')
        # TODO: Add an option to overwrite analyses if they already exist
        if not os.path.isfile(output_file):
            cmd = f'python3 analyze_transformations_in_program.py {result_fn} -o={output_file}'
            print(cmd)
            t0 = datetime.now()
            run(cmd, shell=True)
            t1 = datetime.now()
            print(f'transformation analysis time: {t1-t0}')
            sys.stdout.flush()


if __name__ == '__main__':
    main()
