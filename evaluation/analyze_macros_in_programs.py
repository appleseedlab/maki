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
    ap.add_argument('num_threads', type=int)
    args = ap.parse_args()

    # create the results directory
    os.makedirs('./results/', exist_ok=True)

    # evaluate each program
    for p in PROGRAMS:
        p_extracted_path = os.path.join(EXTRACTED_PROGRAMS_DIR, p.name)
        if not os.path.exists(p_extracted_path):
            print(f"warning: skipping {p.name}, directory not found")
            continue

        src_dir = p_extracted_path + '/' + p.src_dir
        dst_dir = f"./results/{p.name}"

        # TODO: add an option to run programs even if results already exist
        if os.path.exists(dst_dir):
            print(f"info: skipping {p.name}, already evaluated")
            continue

        cmd = ' '.join(["python3 analyze_macros_in_program.py ",
                       args.cpp2c_so_path,
                       p_extracted_path,
                       src_dir,
                       dst_dir,
                       str(args.num_threads)])
        print(cmd)
        t0 = datetime.now()
        run(cmd, shell=True).check_returncode()
        t1 = datetime.now()
        evaluation_time = t1 - t0
        print(f'{p.name} macro analysis time: {evaluation_time}')
        sys.stdout.flush()


if __name__ == '__main__':
    main()
