#!/usr/bin/python3

import argparse
import os
import sys
from datetime import datetime
from subprocess import run

from constants import EXTRACTED_PROGRAMS_DIR
from evaluation_programs import PROGRAMS


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('cpp2c_so_path', type=str)
    ap.add_argument('macro_invocation_analysis_time_output_file', type=str)
    ap.add_argument('num_threads', type=int)
    args = ap.parse_args()

    # create the macro_invocation_analyses directory
    os.makedirs('./macro_invocation_analyses/', exist_ok=True)

    # evaluate each program
    with open(args.macro_invocation_analysis_time_output_file, 'w') as ofp:
        print('Program,Time', file=ofp)
        for p in PROGRAMS:
            p_extracted_path = os.path.join(EXTRACTED_PROGRAMS_DIR, p.name)
            if not os.path.exists(p_extracted_path):
                print(f"warning: skipping {p.name}, directory not found")
                continue

            src_dir = p_extracted_path + '/' + p.src_dir
            dst_dir = f"./macro_invocation_analyses/{p.name}"

            # TODO: add an option to run programs even if results already exist
            if os.path.exists(dst_dir):
                print(f"info: skipping {p.name}, already evaluated")
                continue

            cmd = f'./analyze_macro_invocations_in_program.py "{args.cpp2c_so_path}" "{p_extracted_path}" "{src_dir}" "{dst_dir}" {args.num_threads}'
            print(cmd)
            t0 = datetime.now()
            run(cmd, shell=True).check_returncode()
            t1 = datetime.now()
            delta = t1 - t0
            delta_formatted = f'{delta.days}:{delta.seconds // 3600}:{delta.seconds // 60}:{delta.seconds}:{delta.microseconds}'
            print(f'{p.name},{delta_formatted}', file=ofp)
            sys.stdout.flush()
            ofp.flush()


if __name__ == '__main__':
    main()
