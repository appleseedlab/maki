#!/usr/bin/python3

import json
import sys
from collections import defaultdict
from dataclasses import dataclass
from typing import Dict, Set

USAGE_STRING = r'./analyze_results.py RESULTS_FILE'

DELIM = '\t'


@dataclass(frozen=True)
class Macro:
    Name: str
    ValidDefLoc: bool
    DefLocOrError: str


@dataclass(frozen=True)
class Invocation:
    Name: str
    DefinitionLocation: str
    InvocationLocation: str
    ASTKind: str
    TypeSignature: str

    InvocationDepth: int
    NumASTRoots: int
    NumArguments: int

    HasStringification: bool
    HasTokenPasting: bool
    HasAlignedArguments: bool
    HasSameNameAsOtherDeclaration: bool

    IsExpansionControlFlowStmt: bool

    DoesBodyReferenceMacroDefinedAfterMacro: bool
    DoesBodyReferenceDeclDeclaredAfterMacro: bool
    DoesBodyContainDeclRefExpr: bool
    DoesSubexpressionExpandedFromBodyHaveLocalType: bool
    DoesSubexpressionExpandedFromBodyHaveTypeDefinedAfterMacro: bool

    DoesAnyArgumentHaveSideEffects: bool
    DoesAnyArgumentContainDeclRefExpr: bool

    IsHygienic: bool
    IsDefinitionLocationValid: bool
    IsInvocationLocationValid: bool
    IsObjectLike: bool
    IsInvokedInMacroArgument: bool
    IsNamePresentInCPPConditional: bool
    IsExpansionICE: bool

    IsExpansionTypeNull: bool
    IsExpansionTypeAnonymous: bool
    IsExpansionTypeLocalType: bool
    IsExpansionTypeDefinedAfterMacro: bool
    IsExpansionTypeVoid: bool

    IsAnyArgumentTypeNull: bool
    IsAnyArgumentTypeAnonymous: bool
    IsAnyArgumentTypeLocalType: bool
    IsAnyArgumentTypeDefinedAfterMacro: bool
    IsAnyArgumentTypeVoid: bool

    IsInvokedWhereModifiableValueRequired: bool
    IsInvokedWhereAddressableValueRequired: bool
    IsInvokedWhereICERequired: bool

    IsAnyArgumentExpandedWhereModifiableValueRequired: bool
    IsAnyArgumentExpandedWhereAddressableValueRequired: bool
    IsAnyArgumentConditionallyEvaluated: bool
    IsAnyArgumentNeverExpanded: bool
    IsAnyArgumentNotAnExpression: bool


MacroMap = Dict[Macro, Set[Invocation]]


def main():
    if len(sys.argv) != 2:
        print(USAGE_STRING)
        exit(1)
    f = sys.argv[1]

    lines: list[str] = []
    with open(f) as fp:
        lines = fp.readlines()

    # mapping from macros to invocations
    mis: MacroMap = defaultdict(set)
    # set of names of macros inspected by the CPP
    inspected_macro_names = set()
    # set of files included at a local scope
    local_includes = set()

    # src directory, to be initialized during the analysis
    src_dir = ''

    for line in lines:
        line = line.rstrip()

        if line.startswith('Src'):
            _, src_dir = line.split(DELIM)

        elif line.startswith('Define'):
            _, Name, ValidDefLoc, DefLocOrError = line.split(DELIM)
            m = Macro(Name, ValidDefLoc == 'T', DefLocOrError)
            if m not in mis:
                mis[m] = set()

        elif line.startswith('InspectedByCPP'):
            _, Name = line.split(DELIM)
            inspected_macro_names.add(Name)

        elif line.startswith('Include'):
            _, Valid, IncludedFileRealPath = line.split(DELIM)
            if Valid == 'F':
                local_includes.add(IncludedFileRealPath)

        elif line.startswith('Invocation'):
            _, j = line.split(DELIM, 1)
            i = Invocation(**(json.loads(j)))
            m = Macro(i.Name,
                      i.IsInvocationLocationValid,
                      i.DefinitionLocation)
            mis[m].add(i)

    print('defined macros:', len(mis.keys()))

    print('macros defined at invalid locations',
          len([m for m in mis.keys() if not m.ValidDefLoc]))

    src_mis = {m: is_ for m, is_ in mis.items()
               if m.DefLocOrError.startswith(src_dir)}

    print('macros defined in src_dir', len(src_mis))

    print('number of unique src macro invocations',
          sum([len(is_) for is_ in src_mis.values()]))

    print('src invocations at invalid locations',
          sum([len([i for i in is_ if not i.IsInvocationLocationValid])
               for is_ in src_mis.values()]))

    print('nested src invocations',
          sum([len([i for i in is_ if i.InvocationDepth > 0])
               for is_ in src_mis.values()]))

    print('src invocations in macro arguments',
          sum([len([i for i in is_ if i.IsInvokedInMacroArgument])
               for is_ in src_mis.values()]))

    # TODO: double check that these are actually the minimum requirements for
    # transformation
    print('transformable src invocations',
          sum([len([i for i in is_
                    if all([i.NumASTRoots == 1,
                           i.HasAlignedArguments,
                           not (not i.IsObjectLike
                                and i.IsAnyArgumentNotAnExpression)])])
               for is_ in src_mis.values()]))


if __name__ == '__main__':
    main()
