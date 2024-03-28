#!/usr/bin/python3

import os
import sys
import urllib.request
from datetime import datetime as dt
from shutil import move, unpack_archive
from subprocess import run
from sys import stderr
from typing import Set

from constants import ARCHIVED_PROGRAMS_DIR, EXTRACTED_PROGRAMS_DIR
from evaluation_programs import PROGRAMS

USAGE_STRING = './setup_programs.py (all)|(PROGRAM_NAME+)'


def main():
    given_programs: Set[str] = set()
    # Check if the user wants to set up all programs or only a given set
    # of programs
    if any([len(sys.argv) < 2,
            len(sys.argv) > 2 and sys.argv[1] == 'all']):
        print(USAGE_STRING, file=sys.stderr)
        exit(1)
    else:
        if len(sys.argv) == 2 and sys.argv[1] == 'all':
            # Set up all programs
            pass
        else:
            # Set up a given set of programs
            given_programs = set(sys.argv[1:])

    for p in PROGRAMS:
        # Check if we should skip setting up this program
        if given_programs and p.name not in given_programs:
            continue

        p_archive_path = os.path.join(ARCHIVED_PROGRAMS_DIR, p.archive_file)

        # Download the program zip file if we do not already have it
        if not os.path.exists(p_archive_path):

            # Create the archive dir if it does not exist
            if not os.path.exists(ARCHIVED_PROGRAMS_DIR):
                os.makedirs(ARCHIVED_PROGRAMS_DIR, exist_ok=True)

            print(f'Downloading {p.name} from {p.archive_url}', file=stderr)

            # Download the program's archive
            if p.archive_url.startswith('http'):
                # http(s) download
                urllib.request.urlretrieve(p.archive_url, p_archive_path)
                # with urllib.request.urlopen(p.archive_url) as page:
                #     with open(p_archive_path, 'wb') as fp:
                #         fp.write(page.read())
            elif p.archive_url.startswith('ftp'):
                # ftp download
                # TODO: Use ftp lib instead of relying on wget
                run(f'wget --no-passive {p.archive_url}', shell=True)
                move(p.archive_file, ARCHIVED_PROGRAMS_DIR)

            print(f'Finished downloading {p.name}', file=stderr)

        p_extracted_path = os.path.join(EXTRACTED_PROGRAMS_DIR, p.name)

        # Setup the program only if it has not been set up before
        if not os.path.exists(p_extracted_path):

            # Create the extracted archive dir if it does not exist
            if not os.path.exists(EXTRACTED_PROGRAMS_DIR):
                os.makedirs(EXTRACTED_PROGRAMS_DIR, exist_ok=True)

            # Create a fresh extracted archive
            unpack_archive(p_archive_path, EXTRACTED_PROGRAMS_DIR)

            # Save the current directory so we can move back to it after
            # evaluating this program
            evaluation_dir = os.getcwd()

            # Configure program
            print(f'Configuring {p.name}', file=stderr)
            os.chdir(p_extracted_path)
            configure_time_start = dt.now()
            p.configure().check_returncode()
            configure_time_end = dt.now()
            print(f'Finished configuring {p.name}', file=stderr)
            configure_time = configure_time_end - configure_time_start

            # Build program and generate compile_commands.json
            print(f'Building {p.name}', file=stderr)
            build_start_time = dt.now()
            p.build().check_returncode()
            build_end_time = dt.now()
            print(f'Finished building {p.name}', file=stderr)
            build_time = build_end_time - build_start_time

            # Go back to the evaluation directory
            os.chdir(evaluation_dir)


if __name__ == '__main__':
    main()
