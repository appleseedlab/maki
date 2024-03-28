#!/usr/bin/python3

import argparse
import json
import sys
from dataclasses import asdict
from itertools import chain
from typing import Callable, List, Set

from analysis import Analysis, MacroStat, definition_stat, invocation_stat
from macros import Invocation, Macro, PreprocessorData
from predicates.argument_altering import aa_invocation
from predicates.call_site_context_altering import csca_invocation
from predicates.declaration_altering import da_invocation
from predicates.interface_equivalent import ie_def
from predicates.mennie import mennie_def
from predicates.metaprogramming import mp_invocation
from predicates.thunkizing import thunkizing_invocation
from predicates.property_categories import *

DELIM = '\t'


TRANSFORMATIONS = [aa_invocation, da_invocation,
                   csca_invocation, mp_invocation,
                   thunkizing_invocation]


InvocationPredicate = Callable[[Invocation, PreprocessorData], bool]


def only(i: Invocation,
         pd: PreprocessorData,
         p: InvocationPredicate,
         ps: List[InvocationPredicate]):
    '''
    Returns true if the predicate p is the only one that this
    transformation satisfies.
    '''
    assert p in ps
    satisfied = [p for p in ps if p(i, pd)]
    return satisfied == [p]


def mdefs_only_p(pd: PreprocessorData,
                 p: InvocationPredicate,
                 ps: List[InvocationPredicate]):
    return definition_stat(
        pd,
        lambda m, pd: all([only(i, pd, p, ps) for i in pd.mm[m]]))


def mdefs_at_least_p(pd, p):
    return definition_stat(pd,
                           lambda m, pd: any([p(i, pd) for i in pd.mm[m]]))


def invocations_only_p(pd, p, ps):
    return invocation_stat(pd, lambda i, pd_: only(i, pd_, p, ps))


def invocations_at_least_p(pd, p):
    return invocation_stat(pd, p)


def easy_to_transform_invocation(i: Invocation,
                                 pd: PreprocessorData,
                                 ie_invocations: Set[Invocation]):
    return ((i in ie_invocations) or
            ((aa_invocation(i, pd) or da_invocation(i, pd)) and
             (not csca_invocation(i, pd)) and
             (not thunkizing_invocation(i, pd)) and
             (not mp_invocation(i, pd))))


def easy_to_transform_definition(m: Macro,
                                 pd: PreprocessorData,
                                 ie_invocations: Set[Invocation]):
    return all([
        easy_to_transform_invocation(i, pd, ie_invocations)
        for i in pd.mm[m]
    ])


def avg_or_zero(values):
    return round(sum(values) / len(values), 2) if values else 0


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('results_file', type=str)
    ap.add_argument('-o', '--output_file')
    args = ap.parse_args()

    lines: list[str] = []
    with open(args.results_file) as fp:
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
                      i.IsDefinitionLocationValid,
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

    # tlna_src_pd only records preprocessor data about top-level,
    # non-argument source macros
    tlna_src_pd = PreprocessorData(
        {m: is_ for m, is_ in src_pd.mm.items()
         if all([i.IsTopLevelNonArgument for i in is_])},
        src_pd.inspected_macro_names,
        src_pd.local_includes
    )

    # # # Programs I don't have examples of yet:
    # # # cvs, enscript, flex, m4, perl, rcs

    # from pprint import pprint
    # for m, is_ in tlna_src_pd.mm.items():
    #     for i in is_:
    #         # if i.DoesAnyArgumentHaveSideEffects:
    #         # if csca_invocation(i, tlna_src_pd):
    #         # if i.IsAnyArgumentExpandedWhereAddressableValueRequired:
    #         # if i.IsExpansionTypeLocalType or i.IsAnyArgumentTypeLocalType:
    #         # if i.IsAnyArgumentTypeVoid:
    #         if i.ASTKind == 'Decl':
    #             pprint(i)
    # return

    # ie_pd only records preprocessor data about interface-equivalent
    # macros
    ie_pd = PreprocessorData(
        {m: is_ for m, is_ in tlna_src_pd.mm.items()
         if ie_def(m, tlna_src_pd)},
        tlna_src_pd.inspected_macro_names,
        tlna_src_pd.local_includes
    )

    ie_invocations = set(chain(*ie_pd.mm.values()))

    # mennie_pd only records preprocessor data about macros which mennie
    # could have transformed (I believe only interface-equivalent
    # object-like macros which could be turned into variables)
    mennie_pd = PreprocessorData(
        {m: is_ for m, is_ in tlna_src_pd.mm.items()
         if mennie_def(m, tlna_src_pd)},
        tlna_src_pd.inspected_macro_names,
        tlna_src_pd.local_includes
    )

    # tlna_src_pd_easy only records preprocessor data about macros with
    # easy to transform definitions
    tlna_src_pd_easy = PreprocessorData(
        {m: is_ for m, is_ in tlna_src_pd.mm.items()
         if easy_to_transform_definition(m, tlna_src_pd, ie_invocations)},
        tlna_src_pd.inspected_macro_names,
        tlna_src_pd.local_includes
    )

    # tlna_src_pd_hard only records preprocessor data about macros with
    # hard to transform definitions
    tlna_src_pd_hard = PreprocessorData(
        {m: is_ for m, is_ in tlna_src_pd.mm.items()
         if not easy_to_transform_definition(m, tlna_src_pd, ie_invocations)},
        tlna_src_pd.inspected_macro_names,
        tlna_src_pd.local_includes
    )

    a = Analysis(
        defined_macros=definition_stat(pd, lambda _m, _pd: True),
        macros_defined_at_valid_src_locs=definition_stat(
            src_pd, lambda m, _pd: True),
        macros_defined_at_valid_src_locs_with_only_top_level_non_argument_invocations=definition_stat(
            tlna_src_pd, lambda m, _pd: True),

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
        top_level_argument_src_invocations=invocation_stat(
            src_pd,
            lambda i, _pd: i.InvocationDepth == 0 and i.IsInvokedInMacroArgument),
        top_level_non_argument_src_invocations=invocation_stat(
            tlna_src_pd, lambda i, _pd: True),

        top_level_non_argument_src_invocations_with_semantic_data=invocation_stat(
            tlna_src_pd, lambda i, _pd: i.HasSemanticData),

        interface_equivalent_src_definitions=definition_stat(
            ie_pd, lambda _m, ie_pd: True),
        # We have to compute the invocation stat for
        # interface-equivalent macro invocations somewhat differently
        # from the rest because in order for an invocation to truly
        # be interface-equivalent, all its definition's corresponding
        # invocations must be interface-equivalent as well
        top_level_non_argument_interface_equivalent_src_invocations=invocation_stat(
            ie_pd, lambda _m, _pd: True),

        mennie_definitions=definition_stat(mennie_pd, lambda _m, _pd: True),
        mennie_invocations=invocation_stat(mennie_pd, lambda _m, _pd: True),

        src_definitions_with_only_argument_altering_invocations=mdefs_only_p(
            tlna_src_pd, aa_invocation, TRANSFORMATIONS),
        src_definitions_with_any_argument_altering_invocations=mdefs_at_least_p(
            tlna_src_pd, aa_invocation),
        top_level_non_argument_src_invocations_that_are_only_argument_altering=invocations_only_p(
            tlna_src_pd, aa_invocation, TRANSFORMATIONS),
        top_level_non_argument_src_invocations_that_are_at_least_argument_altering=invocations_at_least_p(
            tlna_src_pd, aa_invocation),

        src_definitions_with_only_declaration_altering_invocations=mdefs_only_p(
            tlna_src_pd, da_invocation, TRANSFORMATIONS),
        src_definitions_with_any_declaration_altering_invocations=mdefs_at_least_p(
            tlna_src_pd, da_invocation),
        top_level_non_argument_src_invocations_that_are_only_declaration_altering=invocations_only_p(
            tlna_src_pd, da_invocation, TRANSFORMATIONS),
        top_level_non_argument_src_invocations_that_are_at_least_declaration_altering=invocations_at_least_p(
            tlna_src_pd, da_invocation),

        src_definitions_with_only_call_site_context_altering_invocations=mdefs_only_p(
            tlna_src_pd, csca_invocation, TRANSFORMATIONS),
        src_definitions_with_any_call_site_context_altering_invocations=mdefs_at_least_p(
            tlna_src_pd, csca_invocation),
        top_level_non_argument_src_invocations_that_are_only_call_site_context_altering=invocations_only_p(
            tlna_src_pd, csca_invocation, TRANSFORMATIONS),
        top_level_non_argument_src_invocations_that_are_at_least_call_site_context_altering=invocations_at_least_p(
            tlna_src_pd, csca_invocation),

        src_definitions_with_only_thunkizing_invocations=mdefs_only_p(
            tlna_src_pd, thunkizing_invocation, TRANSFORMATIONS),
        src_definitions_with_any_thunkizing_invocations=mdefs_at_least_p(
            tlna_src_pd, thunkizing_invocation),
        top_level_non_argument_src_invocations_that_are_only_thunkizing=invocations_only_p(
            tlna_src_pd, thunkizing_invocation, TRANSFORMATIONS),
        top_level_non_argument_src_invocations_that_are_at_least_thunkizing=invocations_at_least_p(
            tlna_src_pd, thunkizing_invocation),

        src_definitions_with_only_metaprogramming_invocations=mdefs_only_p(
            tlna_src_pd, mp_invocation, TRANSFORMATIONS),
        src_definitions_with_any_metaprogramming_invocations=mdefs_at_least_p(
            tlna_src_pd, mp_invocation),
        top_level_non_argument_src_invocations_that_are_only_metaprogramming=invocations_only_p(
            tlna_src_pd, mp_invocation, TRANSFORMATIONS),
        top_level_non_argument_src_invocations_that_are_at_least_metaprogramming=invocations_at_least_p(
            tlna_src_pd, mp_invocation),

        src_definitions_that_are_easy_to_transform=definition_stat(
            tlna_src_pd,
            lambda m, pd: easy_to_transform_definition(m, pd, ie_invocations)),
        top_level_non_argument_src_invocations_that_are_easy_to_transform=invocation_stat(
            tlna_src_pd,
            lambda i, pd: easy_to_transform_invocation(i, pd, ie_invocations)),

        src_definitions_that_are_hard_to_transform=definition_stat(
            tlna_src_pd,
            lambda m, pd: not easy_to_transform_definition(m, pd, ie_invocations)),
        top_level_non_argument_src_invocations_that_are_hard_to_transform=invocation_stat(
            tlna_src_pd,
            lambda i, pd: not easy_to_transform_invocation(i, pd, ie_invocations)),

        avg_top_level_non_argument_src_invocations_per_easy_to_transform_definition=MacroStat(
            olms=avg_or_zero([len(is_)
                              for m, is_ in tlna_src_pd_easy.mm.items()
                              if m.IsObjectLike]),
            flms=avg_or_zero([len(is_)
                              for m, is_ in tlna_src_pd_easy.mm.items()
                              if m.IsFunctionLike]),
            total=avg_or_zero([len(is_)
                              for m, is_ in tlna_src_pd_easy.mm.items()])),
        avg_top_level_non_argument_src_invocations_per_hard_to_transform_definition=MacroStat(
            olms=avg_or_zero([len(is_)
                              for m, is_ in tlna_src_pd_hard.mm.items()
                              if m.IsObjectLike]),
            flms=avg_or_zero([len(is_)
                              for m, is_ in tlna_src_pd_hard.mm.items()
                              if m.IsFunctionLike]),
            total=avg_or_zero([len(is_)
                              for m, is_ in tlna_src_pd_hard.mm.items()])),

        src_definitions_with_only_decl_invocations=definition_stat(
            tlna_src_pd,
            lambda m, pd: all([i.ASTKind == 'Decl' for i in pd.mm[m]])),

        top_level_non_argument_decl_invocations=invocation_stat(
            tlna_src_pd,
            lambda i, pd: i.ASTKind == 'Decl'),

        src_definitions_with_only_stmt_invocations=definition_stat(
            tlna_src_pd,
            lambda m, pd: all([i.ASTKind == 'Stmt' for i in pd.mm[m]])),

        top_level_non_argument_stmt_invocations=invocation_stat(
            tlna_src_pd,
            lambda i, pd: i.ASTKind == 'Stmt'),

        src_definitions_with_only_expr_invocations=definition_stat(
            tlna_src_pd,
            lambda m, pd: all([i.ASTKind == 'Expr' for i in pd.mm[m]])),

        top_level_non_argument_expr_invocations=invocation_stat(
            tlna_src_pd,
            lambda i, pd: i.ASTKind == 'Expr'),

        src_definitions_with_only_type_loc_invocations=definition_stat(
            tlna_src_pd,
            lambda m, pd: all([i.ASTKind == 'TypeLoc' for i in pd.mm[m]])),

        top_level_non_argument_type_loc_invocations=invocation_stat(
            tlna_src_pd,
            lambda i, pd: i.ASTKind == 'TypeLoc'),

        src_definitions_with_mixed_syntax_invocations=definition_stat(
            tlna_src_pd,
            lambda m, pd: len({i.ASTKind for i in pd.mm[m]}) != 1),


        src_definitions_with_invocations_that_only_satisfy_syntactic_properties=mdefs_only_p(
            tlna_src_pd, syntactic, PROPERTY_CATEGORIES),
        src_definitions_with_any_invocation_that_satisfies_syntactic_properties=mdefs_at_least_p(
            tlna_src_pd, syntactic),

        top_level_non_argument_src_invocations_that_only_satisfy_syntactic_properties=invocations_only_p(
            tlna_src_pd, syntactic, PROPERTY_CATEGORIES),

        top_level_non_argument_src_invocations_that_at_least_satisfy_syntactic_properties=invocations_at_least_p(
            tlna_src_pd, syntactic),


        src_definitions_with_invocations_that_only_satisfy_scoping_properties=mdefs_only_p(
            tlna_src_pd, scoping, PROPERTY_CATEGORIES),
        src_definitions_with_any_invocation_that_satisfies_scoping_properties=mdefs_at_least_p(
            tlna_src_pd, scoping),

        top_level_non_argument_src_invocations_that_only_satisfy_scoping_properties=invocations_only_p(
            tlna_src_pd, scoping, PROPERTY_CATEGORIES),

        top_level_non_argument_src_invocations_that_at_least_satisfy_scoping_properties=invocations_at_least_p(
            tlna_src_pd, scoping),

        src_definitions_with_invocations_that_only_satisfy_typing_properties=mdefs_only_p(
            tlna_src_pd, typing, PROPERTY_CATEGORIES),
        src_definitions_with_any_invocation_that_satisfies_typing_properties=mdefs_at_least_p(
            tlna_src_pd, typing),

        top_level_non_argument_src_invocations_that_only_satisfy_typing_properties=invocations_only_p(
            tlna_src_pd, typing, PROPERTY_CATEGORIES),

        top_level_non_argument_src_invocations_that_at_least_satisfy_typing_properties=invocations_at_least_p(
            tlna_src_pd, typing),

        src_definitions_with_invocations_that_only_satisfy_calling_convention_properties=mdefs_only_p(
            tlna_src_pd, calling_convention, PROPERTY_CATEGORIES),
        src_definitions_with_any_invocation_that_satisfies_calling_convention_properties=mdefs_at_least_p(
            tlna_src_pd, calling_convention),

        top_level_non_argument_src_invocations_that_only_satisfy_calling_convention_properties=invocations_only_p(
            tlna_src_pd, calling_convention, PROPERTY_CATEGORIES),

        top_level_non_argument_src_invocations_that_at_least_satisfy_calling_convention_properties=invocations_at_least_p(
            tlna_src_pd, calling_convention),

        src_definitions_with_invocations_that_only_satisfy_language_specific_properties=mdefs_only_p(
            tlna_src_pd, language_specific, PROPERTY_CATEGORIES),
        src_definitions_with_any_invocation_that_satisfies_language_specific_properties=mdefs_at_least_p(
            tlna_src_pd, language_specific),

        top_level_non_argument_src_invocations_that_only_satisfy_language_specific_properties=invocations_only_p(
            tlna_src_pd, language_specific, PROPERTY_CATEGORIES),

        top_level_non_argument_src_invocations_that_at_least_satisfy_language_specific_properties=invocations_at_least_p(
            tlna_src_pd, language_specific),

        src_definitions_with_invocations_that_satisfy_no_properties=definition_stat(
            tlna_src_pd, lambda m, pd: all([
                not any([
                    p(i, pd)
                    for p in PROPERTY_CATEGORIES
                ])
                for i in pd.mm[m]
            ])),
        src_definitions_with_any_invocation_that_satisfies_no_properties=definition_stat(
            tlna_src_pd, lambda m, pd: any([
                not any([
                    p(i, pd)
                    for p in PROPERTY_CATEGORIES
                ])
                for i in pd.mm[m]
            ])),
        top_level_non_argument_src_invocations_that_satisfy_no_properties=invocation_stat(
            tlna_src_pd, lambda i, pd: not any([
                p(i, pd)
                for p in PROPERTY_CATEGORIES
            ])),
    )

    if args.output_file:
        with open(args.output_file, 'w', encoding='utf-8') as ofp:
            json.dump(asdict(a), ofp, indent=4)
    else:
        json.dump(asdict(a), sys.stdout, indent=4)


if __name__ == '__main__':
    main()
