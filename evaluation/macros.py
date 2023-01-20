from collections import defaultdict
from dataclasses import dataclass, field
from typing import DefaultDict, Literal, Set


@dataclass(frozen=True)
class Macro:
    Name: str
    IsObjectLike: bool
    IsDefLocValid: bool
    DefLocOrError: str

    def defined_in(self, dir: str):
        '''Returns true if the macro was defined in the given dir'''
        return self.IsDefLocValid and self.DefLocOrError.startswith(dir)

    @property
    def IsFunctionLike(self) -> bool:
        return not self.IsObjectLike


@dataclass(frozen=True)
class Invocation:
    Name: str
    DefinitionLocation: str
    InvocationLocation: str
    ASTKind: Literal['Decl', 'Stmt', 'TypeLoc', 'Expr']
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

    @property
    def DefinitionLocationFilename(self) -> str:
        if not self.IsDefinitionLocationValid:
            return self.DefinitionLocation
        else:
            file, _line, _col = self.DefinitionLocation.split(':')
            return file

    @property
    def IsFunctionLike(self) -> bool:
        return not self.IsObjectLike

    @property
    def HasValidSemanticData(self) -> bool:
        return all([self.InvocationDepth == 0,
                    not self.IsInvokedInMacroArgument,
                    self.IsInvocationLocationValid,
                    self.IsDefinitionLocationValid])

    @property
    def IsAligned(self) -> bool:
        assert self.HasValidSemanticData
        return all([self.HasValidSemanticData,
                    self.NumASTRoots == 1,
                    self.HasAlignedArguments])

    @property
    def CanBeTurnedIntoEnum(self) -> bool:
        return self.IsObjectLike and self.IsExpansionICE

    @property
    def CanBeTurnedIntoVariable(self) -> bool:
        assert self.HasValidSemanticData
        return all([
            self.ASTKind == 'Expr',
            not self.IsInvokedWhereICERequired,
            not self.IsExpansionTypeNull,
            not self.IsExpansionTypeVoid
        ])

    @property
    def CanBeTurnedIntoFunction(self) -> bool:
        assert self.HasValidSemanticData
        return all([
            (self.ASTKind == 'Stmt' or self.ASTKind == 'Expr'),
            not self.IsInvokedWhereICERequired,
            (self.ASTKind == 'Stmt' or
             (self.ASTKind == 'Expr' and not self.IsExpansionTypeNull))
        ])

    @property
    def CanBeTurnedIntoAFunctionOrVariable(self) -> bool:
        return (self.CanBeTurnedIntoFunction or
                self.CanBeTurnedIntoVariable)

    @property
    def ExploitsDynamicScoping(self) -> bool:
        assert self.HasValidSemanticData
        return any([
            not self.IsHygienic,
            self.IsInvokedWhereModifiableValueRequired,
            self.IsInvokedWhereAddressableValueRequired,
            self.IsAnyArgumentExpandedWhereModifiableValueRequired,
            self.IsAnyArgumentExpandedWhereAddressableValueRequired])

    @property
    def ExploitsMacroDeclarationSemantics(self) -> bool:
        assert self.HasValidSemanticData
        return any([
            self.HasSameNameAsOtherDeclaration,
            self.DoesBodyReferenceMacroDefinedAfterMacro,
            self.DoesBodyReferenceDeclDeclaredAfterMacro,
            self.DoesSubexpressionExpandedFromBodyHaveLocalType,
            self.DoesSubexpressionExpandedFromBodyHaveTypeDefinedAfterMacro,
            self.IsExpansionTypeAnonymous,
            self.IsAnyArgumentTypeDefinedAfterMacro,
            self.IsAnyArgumentTypeLocalType,
        ])

    @property
    def InvolvesMetaprogramming(self) -> bool:
        return self.HasStringification or self.HasTokenPasting


MacroMap = DefaultDict[Macro, Set[Invocation]]


@dataclass
class PreprocessorData:
    mm: MacroMap = field(default_factory=lambda: defaultdict(set))
    inspected_macro_names: Set[str] = field(default_factory=set)
    local_includes: Set[str] = field(default_factory=set)
