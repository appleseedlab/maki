#!/usr/bin/python3

import json
import sys
from dataclasses import asdict
from typing import Callable

from analysis import Analysis, definition_stat, invocation_stat
from macros import Invocation, Macro, PreprocessorData
from predicates.argument_altering import aa_invocation
from predicates.declaration_altering import da_invocation
from predicates.call_site_context_altering import csca_invocation
from predicates.interface_equivalent import ie_def
from predicates.metaprogramming import mp_invocation
from predicates.thunkizing import thunkizing_invocation

ANALYSES_DIR = r'ANALYSES'
DELIM = '\t'
USAGE_STRING = r'./analyze_results.py RESULTS_FILE'


def only(i: Invocation,
         pd: PreprocessorData,
         t: Callable[[Invocation, PreprocessorData], bool]):
    transformations = [aa_invocation, da_invocation,
                       csca_invocation, mp_invocation,
                       thunkizing_invocation]
    '''
    Returns true if the transformation t is the only one that works
    for this invocation.
    '''
    assert t in transformations
    trs = [tr for tr in transformations if tr(i, pd)]
    return trs == [t]


def mdefs_only_t(pd, t):
    return definition_stat(
        pd,
        lambda m, pd: all([only(i, pd, t) for i in pd.mm[m]]))


def mdefs_at_least_t(pd, t):
    return definition_stat(pd,
                           lambda m, pd: any([t(i, pd) for i in pd.mm[m]]))


def invocations_only_t(pd, t):
    return invocation_stat(pd, lambda i, pd_: only(i, pd_, t))


def invocations_at_least_t(pd, t):
    return invocation_stat(pd, t)


def main():
    if len(sys.argv) != 2:
        print(USAGE_STRING, file=sys.stderr)
        exit(1)
    f = sys.argv[1]

    lines: list[str] = []
    with open(f) as fp:
        lines = fp.readlines()

    pd = PreprocessorData()

    # src directory, to be initialized during the analysis
    src_dir = ''

    for line in lines:
        line = line.rstrip()

        if line.startswith('Src'):
            _, src_dir = line.split(DELIM)

        elif line.startswith('Define'):
            _, Name, IsObjectLike, IsDefLocValid, DefLocOrError = line.split(
                DELIM)
            m = Macro(Name, IsObjectLike == 'T',
                      IsDefLocValid == 'T', DefLocOrError)
            if m not in pd.mm:
                pd.mm[m] = set()

        elif line.startswith('InspectedByCPP'):
            _, Name = line.split(DELIM)
            pd.inspected_macro_names.add(Name)

        elif line.startswith('Include'):
            _, Valid, IncludedFileRealPath = line.split(DELIM)
            if Valid == 'F':
                pd.local_includes.add(IncludedFileRealPath)

        elif line.startswith('Invocation'):
            _, j = line.split(DELIM, 1)
            i = Invocation(**(json.loads(j)))
            m = Macro(i.Name,
                      i.IsObjectLike,
                      i.IsInvocationLocationValid,
                      i.DefinitionLocation)
            # Only record unique invocations - two invocations may have the same
            # location if they are the same nested invocation
            if all([j.InvocationLocation != i.InvocationLocation for j in pd.mm[m]]):
                pd.mm[m].add(i)

    # src_pd only records preprocessor data about source macros
    src_pd = PreprocessorData(
        {m: is_ for m, is_ in pd.mm.items() if m.defined_in(src_dir)},
        pd.inspected_macro_names,
        pd.local_includes
    )

    a = Analysis(
        defined_macros=definition_stat(pd, lambda _m, _pd: True),
        macros_defined_at_valid_src_locs=definition_stat(
            src_pd, lambda m, _pd: True),

        # Can just return true because we only record invocations at unique
        # locations anyway
        src_invocations_at_unique_locations=invocation_stat(
            src_pd, lambda _i, _pd: True),
        src_invocations_at_unique_valid_locations=invocation_stat(
            src_pd, lambda i, _pd: i.IsInvocationLocationValid),
        src_invocations_at_unique_invalid_locations=invocation_stat(
            src_pd, lambda i, _pd: not i.IsInvocationLocationValid),

        nested_argument_src_invocations=invocation_stat(
            src_pd,
            lambda i, _pd: i.InvocationDepth > 0 and i.IsInvokedInMacroArgument),
        nested_non_argument_src_invocations=invocation_stat(
            src_pd,
            lambda i, _pd: i.InvocationDepth > 0 and not i.IsInvokedInMacroArgument),
        toplevel_argument_src_invocations=invocation_stat(
            src_pd,
            lambda i, _pd: i.InvocationDepth == 0 and i.IsInvokedInMacroArgument),
        toplevel_non_argument_src_invocations=invocation_stat(
            src_pd,
            lambda i, _pd: i.InvocationDepth == 0 and not i.IsInvokedInMacroArgument),

        aligned_src_invocations=invocation_stat(
            src_pd, lambda i, _pd: i.HasValidSemanticData and i.IsAligned),

        interface_equivalent_src_definitions=definition_stat(src_pd, ie_def),
        # We have to compute the invocation stat for
        # interface-equivalent macro invocations somewhat differently
        # from the rest because in order for an invocation to truly
        # be interface-equivalent, all its definition's corresponding
        # invocations must be interface-equivalent as well
        interface_equivalent_src_invocations=invocation_stat(
            PreprocessorData({m: is_ for m, is_ in src_pd.mm.items()
                              if ie_def(m, src_pd)},
                             src_pd.inspected_macro_names,
                             src_pd.local_includes),
            lambda _m, _pd: True),

        src_definitions_with_only_argument_altering_invocations=mdefs_only_t(
            src_pd, aa_invocation),
        src_definitions_with_any_argument_altering_invocations=mdefs_at_least_t(
            src_pd, aa_invocation),
        toplevel_non_argument_src_invocations_that_are_only_argument_altering=invocations_only_t(
            src_pd, aa_invocation),
        toplevel_non_argument_src_invocations_that_are_at_least_argument_altering=invocations_at_least_t(
            src_pd, aa_invocation),

        src_definitions_with_only_declaration_altering_invocations=mdefs_only_t(
            src_pd, da_invocation),
        src_definitions_with_any_declaration_altering_invocations=mdefs_at_least_t(
            src_pd, da_invocation),
        toplevel_non_argument_src_invocations_that_are_only_declaration_altering=invocations_only_t(
            src_pd, da_invocation),
        toplevel_non_argument_src_invocations_that_are_at_least_declaration_altering=invocations_at_least_t(
            src_pd, da_invocation),

        src_definitions_with_only_call_site_context_altering_invocations=mdefs_only_t(
            src_pd, csca_invocation),
        src_definitions_with_any_call_site_context_altering_invocations=mdefs_at_least_t(
            src_pd, csca_invocation),
        toplevel_non_argument_src_invocations_that_are_only_call_site_context_altering=invocations_only_t(
            src_pd, csca_invocation),
        toplevel_non_argument_src_invocations_that_are_at_least_call_site_context_altering=invocations_at_least_t(
            src_pd, csca_invocation),

        src_definitions_with_only_thunkizing_invocations=mdefs_only_t(
            src_pd, thunkizing_invocation),
        src_definitions_with_any_thunkizing_invocations=mdefs_at_least_t(
            src_pd, thunkizing_invocation),
        toplevel_non_argument_src_invocations_that_are_only_thunkizing=invocations_only_t(
            src_pd, thunkizing_invocation),
        toplevel_non_argument_src_invocations_that_are_at_least_thunkizing=invocations_at_least_t(
            src_pd, thunkizing_invocation),

        src_definitions_with_only_metaprogramming_invocations=mdefs_only_t(
            src_pd, mp_invocation),
        src_definitions_with_any_metaprogramming_invocations=mdefs_at_least_t(
            src_pd, mp_invocation),
        toplevel_non_argument_src_invocations_that_are_only_metaprogramming=invocations_only_t(
            src_pd, mp_invocation),
        toplevel_non_argument_src_invocations_that_are_at_least_metaprogramming=invocations_at_least_t(
            src_pd, mp_invocation),

    )

    json.dump(asdict(a), sys.stdout, indent=4)


if __name__ == '__main__':
    main()
