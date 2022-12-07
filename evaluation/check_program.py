#!/usr/bin/python3

import json
import os
import subprocess
import sys
from dataclasses import dataclass
from itertools import repeat
from multiprocessing.pool import ThreadPool
from typing import List


@dataclass
class CompileCommand:
    arguments: str
    directory: str
    file: str


ARGS = [
    'CPP2C_SO_PATH',
    'PROGRAM_DIR',
    'SRC_DIR',
    'DST_DIR'
]

USAGE_STRING = './check_program.py ' + ' '.join(ARGS)

DELIM = "\t"


def removeprefix(s: str, t: str):
    '''Custom removeprefix function for Python 3.8.10 support'''
    i = s.find(t)
    if i == -1:
        return s
    return s[i+len(t):]


def cpp2c(cpp2c_so_path: str,
          cc: CompileCommand,
          src_dir: str,
          dst_path: str,
          i: List[int], n: int) -> None:
    '''
    Runs Cpp2C on the program that the given compile_commands.json file
    comprises in the given src_dir, and prints the results to the outdir

    Parameters:
        cpp2c_so_path:  the path to the built cpp2c shared object file
        cc:             a compile command
        src_dir:        the src directory of the analyzed program
        dst_path:       the path of the file to write cpp2c's results to
        i:              a list containing a single integer, the current number of
                        files processed so far
        n:              the total number of files to process
    '''

    clang_unknown_args = {
        # error causing
        '-mpreferred-stack-boundary',
        '-fconserve-stack',
        '-fno-allow-store-data-races',
        '-flto=auto',

        # warning causing
        '-Wno-format-truncation',
        '-Wno-format-overflow',
        '-Wno-unused-but-set-variable',
        '-Wcast-function-type',
        '-Wno-stringop-truncation',
        '-Wno-stringop-overflow',
        '-Wno-restrict',
        '-Wno-maybe-uninitialized',
        '-Wno-alloc-size-larger-than',
        '-Wimplicit-fallthrough',
        '-Werror=designated-init',
        '-Wno-packed-not-aligned',
        '-Wlogical-op',
        '-Wno-aggressive-loop-optimizations',
    }

    args: list[str] = [arg for arg in cc.arguments
                       if not any([arg.startswith(ua) for ua in clang_unknown_args])]
    # use clang-11
    args[0] = 'clang-11'
    # pass cpp2c plugin shared library file
    args.insert(1, f'-fplugin={cpp2c_so_path}')
    # at the very end, specify that we are only doing syntactic analysis so as to
    # not waste time compiling
    args.insert(-1, '-fsyntax-only')

    fullpath = os.path.realpath(os.path.join(cc.directory, cc.file))
    with open(dst_path, 'w') as ofp:
        print(f'Analyzing {fullpath} ({os.path.getsize(fullpath)} bytes)')
        # print header information about the analysis file
        print(f'Src{DELIM}{src_dir}', file=ofp)
        ofp.flush()
        # change to the directory, then run cpp2c
        cmd = f"cd {cc.directory} && {' '.join(args)}"
        p = subprocess.run(cmd, shell=True, text=True, stdout=ofp)
        if p.stderr:
            print(p.stderr)
        p.check_returncode()

    i[0] += 1
    print(f'{i[0]} / {n} files analyzed', file=sys.stderr)


def main():
    if len(sys.argv) != len(ARGS) + 1:
        print(USAGE_STRING)
        exit(1)

    cpp2c_so_path, program_dir, src_dir, dst_dir = [
        os.path.abspath(a) for a in sys.argv[1:]
    ]

    os.chdir(program_dir)

    if not os.path.isfile('compile_commands.json'):
        print('error: compile_commands.json not found')
        exit(1)

    # load compile commands.json
    fp = open(r'compile_commands.json')
    ccs = [CompileCommand(**cc) for cc in json.load(fp)]
    fp.close()

    # only analyze src files
    ccs = [
        cc for cc in ccs
        if os.path.join(cc.directory, cc.file).startswith(src_dir)
    ]

    # check if a file path contains the delimiter or '"' since this would
    # break the analysis
    # (I doubt this will happen but better to be safe than sorry)
    fullpaths = [os.path.realpath(os.path.join(cc.directory, cc.file))
                 for cc in ccs]
    paths_with_delim_or_double_quote = [
        fp for fp in fullpaths
        if (DELIM in fp) or '"' in fp
    ]
    if paths_with_delim_or_double_quote != []:
        print('some paths contain delimiter or double quote:',
              paths_with_delim_or_double_quote, file=sys.stderr)
        exit(1)

    # analyze every compiled src file in the program
    # we can use multiprocessing because the order in which the facts are
    # emitted does not matter
    i = [0]
    n = len(ccs)
    dst_dirs = [
        os.path.dirname(
            os.path.join(dst_dir,
                         removeprefix(removeprefix(fp, src_dir), ('/'))))
        for fp in fullpaths
    ]
    dst_paths = [
        os.path.join(d,
                     os.path.splitext(os.path.basename(fp))[0] + '.cpp2c')
        for d, fp in zip(dst_dirs, fullpaths)
    ]
    os.makedirs(dst_dir, exist_ok=True)
    for d in dst_dirs:
        os.makedirs(d, exist_ok=True)

    # run cpp2c on all files
    with ThreadPool(8) as pool:
        pool.starmap(cpp2c, zip(repeat(cpp2c_so_path), ccs, repeat(src_dir),
                                dst_paths, repeat(i), repeat(n)))

    # combine all results into a single file
    with open(os.path.join(dst_dir, 'all_results.cpp2c'), 'w') as ofp:
        for dp in dst_paths:
            with open(dp) as ifp:
                ofp.write(ifp.read())


if __name__ == '__main__':
    main()
