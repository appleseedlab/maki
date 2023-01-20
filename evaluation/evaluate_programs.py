#!/usr/bin/python3

import os
import sys
from datetime import datetime
from subprocess import run

from constants import EXTRACTED_PROGRAMS_DIR
from evaluation_programs import PROGRAMS

USAGE_STRING = r"./check_program.py CPP2C_SO_PATH"


def main():

    # check that the user passed the path to the cpp2c shared object file
    if len(sys.argv) != 2:
        print(USAGE_STRING, file=sys.stderr)
        exit(1)

    # extract the cpp2c so path from the command line arguments
    _, cpp2c_so_path = sys.argv

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

        cmd = ' '.join(["./check_program.py ",
                       cpp2c_so_path,
                       p_extracted_path,
                       src_dir,
                       dst_dir])
        print(cmd)
        t0 = datetime.now()
        run(cmd, shell=True).check_returncode()
        t1 = datetime.now()
        evaluation_time = t1 - t0
        print(f'{p.name} evaluation time: {evaluation_time}')


if __name__ == '__main__':
    main()
