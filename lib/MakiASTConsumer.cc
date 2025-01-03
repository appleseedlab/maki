// NOTE:    We can't use TK_IgnoreUnlessSpelledInSource because it ignores
//          paren exprs

#include "MakiASTConsumer.hh"
#include "ASTUtils.hh"
#include "AlignmentMatchers.hh"
#include "DeclCollectorMatchHandler.hh"
#include "DefinitionInfoCollector.hh"
#include "IncludeCollector.hh"
#include "JSONPrinter.hh"
#include "Logging.hh"
#include "MacroExpansionNode.hh"
#include "MacroForest.hh"
#include "MakiFlags.hh"
#include "SourceLocationUtils.hh"
#include "StmtCollectorMatchHandler.hh"
#include <algorithm>
#include <cassert>
#include <clang/AST/ASTContext.h>
#include <clang/AST/Decl.h>
#include <clang/AST/DeclBase.h>
#include <clang/AST/Expr.h>
#include <clang/AST/Stmt.h>
#include <clang/AST/Type.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/Basic/FileEntry.h>
#include <clang/Basic/LLVM.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Lex/Lexer.h>
#include <clang/Lex/Preprocessor.h>
#include <functional>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/Casting.h>
#include <set>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace maki {
using namespace clang::ast_matchers;

// Checks if the included file is a globally included file.
// The first element of the return result if false if not;
// true otherwise.
// The second element is the name of the included file.
std::pair<bool, llvm::StringRef> isGlobalInclude(
    clang::SourceManager &SM, const clang::LangOptions &LO,
    std::pair<clang::OptionalFileEntryRef, clang::SourceLocation> &IEL,
    std::set<llvm::StringRef> &LocalIncludes,
    std::vector<const clang::Decl *> &Decls) {
    auto FE = IEL.first;
    auto HashLoc = IEL.second;

    // Check that the included file has a value
    if (!FE.has_value()) {
        return { false, "<null>" };
    }

    // Check that the included file actually has a name
    auto IncludedFileRealpath = FE->getFileEntry().tryGetRealPathName();
    if (IncludedFileRealpath.empty()) {
        return { false, IncludedFileRealpath };
    }

    // Check that the hash location is valid
    if (HashLoc.isInvalid()) {
        return { false, IncludedFileRealpath };
    }

    // Check that the file the file is included in is valid
    auto IncludedInFID = SM.getFileID(HashLoc);
    if (IncludedInFID.isInvalid()) {
        return { false, IncludedFileRealpath };
    }

    // Check that a file entry exists for the  file the file is included in
    auto IncludedInFE = SM.getFileEntryForID(IncludedInFID);
    if (!IncludedInFE) {
        return { false, IncludedFileRealpath };
    }

    // Check that a real path exists for the file the file is included in
    auto IncludedInRealpath = IncludedInFE->tryGetRealPathName();
    if (IncludedInRealpath.empty()) {
        return { false, IncludedFileRealpath };
    }

    // Check that the file the file is included in is not in turn included
    // in a file included in a non-global scope
    if (LocalIncludes.find(IncludedInRealpath) != LocalIncludes.end()) {
        return { false, IncludedFileRealpath };
    }

    auto HashFLoc = SM.getFileLoc(HashLoc);
    // Check that the file location is valid
    if (HashFLoc.isInvalid()) {
        return { false, IncludedFileRealpath };
    }

    // Check that the include does not appear within the range of any
    // declaration in the file
    if (std::any_of(Decls.begin(), Decls.end(),
                    [&SM, &LO, &HashFLoc](const clang::Decl *D) {
                        auto B = SM.getFileLoc(D->getBeginLoc());
                        auto E = SM.getFileLoc(D->getEndLoc());

                        if (B.isInvalid() || E.isInvalid()) {
                            return false;
                        }

                        // Include the location just after the declaration
                        // to account for semicolons.
                        // If the decl does not have semicolon after it,
                        // that's fine since it would be a non-global
                        // location anyway
                        if (auto Tok = clang::Lexer::findNextToken(E, SM, LO))
                            if (Tok.has_value()) {
                                E = SM.getFileLoc(Tok.value().getEndLoc());
                            }

                        if (E.isInvalid()) {
                            return false;
                        }

                        return clang::SourceRange(B, E).fullyContains(HashFLoc);
                    }))
        return { false, IncludedFileRealpath };

    // Success
    return { true, IncludedFileRealpath };
}

// Checks if the given location is within the declaration range of any of the
// given declarations, either directly or by being #include'd at a local scope.
bool isLocationInAnyDecl(clang::SourceManager &SM, const clang::LangOptions &LO,
                         clang::SourceLocation Location,
                         std::vector<const clang::Decl *> &Decls) {
    std::vector<clang::SourceLocation> LocationsToCheck;
    // Get all the locations that this location was #include'd from, if any.
    for (auto FileID = SM.getFileID(Location); Location.isValid();
         Location = SM.getIncludeLoc(FileID), FileID = SM.getFileID(Location)) {
        LocationsToCheck.push_back(Location);
    }

    // For all locations to check...
    return std::all_of(
        LocationsToCheck.begin(), LocationsToCheck.end(),
        [&SM, &LO, &Decls](clang::SourceLocation Loc) {
            // ..check if the location appears in the source range of any of the
            // given declarations.
            return !std::any_of(
                Decls.begin(), Decls.end(),
                [&SM, &LO, &Loc](const clang::Decl *D) {
                    auto B = SM.getFileLoc(D->getBeginLoc());
                    auto E = SM.getFileLoc(D->getEndLoc());

                    if (B.isInvalid() || E.isInvalid()) {
                        return false;
                    }

                    // If this decl is not for a function, then extend the range
                    // to include the location just after the declaration to
                    // account for semicolons.
                    if (!clang::isa<clang::FunctionDecl>(D)) {
                        if (auto Tok = clang::Lexer::findNextToken(E, SM, LO)) {
                            E = SM.getFileLoc(Tok->getEndLoc());
                        }
                    }

                    if (E.isInvalid()) {
                        return false;
                    }

                    return clang::SourceRange(B, E).fullyContains(Loc);
                });
        });
}

MakiASTConsumer::MakiASTConsumer(clang::CompilerInstance &CI,
                                 MakiFlags Flags_) {
    clang::Preprocessor &PP = CI.getPreprocessor();
    clang::ASTContext &Ctx = CI.getASTContext();
    Flags = Flags_;

    if (!Flags.OnlyCollectDefinitionInfo) {
        MF = new maki::MacroForest(PP, Ctx, Flags);
        IC = new maki::IncludeCollector();
        PP.addPPCallbacks(std::unique_ptr<maki::MacroForest>(MF));
        PP.addPPCallbacks(std::unique_ptr<maki::IncludeCollector>(IC));
    }

    DC = new maki::DefinitionInfoCollector(Ctx, Flags);
    PP.addPPCallbacks(std::unique_ptr<maki::DefinitionInfoCollector>(DC));
}

void MakiASTConsumer::HandleTranslationUnit(clang::ASTContext &Ctx) {
    auto &SM = Ctx.getSourceManager();
    auto &LO = Ctx.getLangOpts();

    // JSON printer for each invocation, definition, etc.
    std::vector<JSONPrinter> printers;

    if (Flags.OnlyCollectDefinitionInfo) {
        for (auto &&Entry : DC->MacroNamesDefinitions) {
            std::string Name = Entry.first;
            std::string DefinitionLocation;
            bool IsDefinitionLocationValid = false;

            auto MD = Entry.second;
            auto MI = MD->getMacroInfo();

            auto DefLoc = MI->getDefinitionLoc();
            std::tie(IsDefinitionLocationValid, DefinitionLocation) =
                tryGetFullSourceLoc(SM, DefLoc);
            JSONPrinter printer{ "Definition" };

            printer.add(
                { { "Name", Name },
                  { "IsDefinitionLocationValid", IsDefinitionLocationValid },
                  { "DefinitionLocation", std::move(DefinitionLocation) } });

            printers.push_back(std::move(printer));
        }

        maki::JSONPrinter::printJSONArray(printers);
        return;
    }

    // Collect all declarations
    std::vector<const clang::Decl *> AllDecls = ({
        MatchFinder Finder;
        DeclCollectorMatchHandler Handler;
        auto Matcher = decl(unless(anyOf(isImplicit(), translationUnitDecl())))
                           .bind("root");
        Finder.addMatcher(Matcher, &Handler);
        Finder.matchAST(Ctx);
        Handler.Decls;
    });

    // Print definition information
    for (auto &&Entry : DC->MacroNamesDefinitions) {
        // String properties
        std::string Body;
        std::string DefinitionLocation;
        std::string EndDefinitionLocation;
        std::string Name = Entry.first;

        // Boolean properties
        bool IsDefinedAtGlobalScope = false;
        bool IsDefinitionLocationValid = false;

        auto MD = Entry.second;
        auto MI = MD->getMacroInfo();
        assert(MI);

        // Append each token to the body, separated by spaces. We separate
        // tokens by spaces to avoid accidentally concatenating tokens in the
        // output (e.g. 1 and 2 becoming 12). We get the spelling of each
        // individual token, and not the spelling of the whole source range
        // using the Lexer, so as to avoid including backslashes and newlines in
        // our emitted definition (which would be annoying for downstream tools
        // to have to escape).
        bool first = true;
        for (auto &token : MI->tokens()) {
            if (!first) {
                Body += " ";
            }
            Body += MF->PP.getSpelling(token);
            first = false;
        }

        auto DefLoc = MI->getDefinitionLoc();

        IsDefinedAtGlobalScope = isLocationInAnyDecl(SM, LO, DefLoc, AllDecls);

        std::tie(IsDefinitionLocationValid, DefinitionLocation) =
            tryGetFullSourceLoc(SM, DefLoc);

        // Set the end definition location.
        //
        // If the macro has no tokens then we can just call the
        // getDefinitionEndLoc() method to get the end of the macro's
        // definition. If the macro has tokens, however, then we can't use this
        // method because it doesn't return correct results when a macro
        // contains a backslash-newline immediately followed by a non-whitespace
        // token. For instance, assuming that the character # corresponds to the
        // first character in a line:
        /*
                #define PLATFORM \
                "LINUX"
        */
        // Then getDefinitionEndLoc() would say that the macro's definition ends
        // on line 1, whereas it really ends on line 2. Instead, we can use the
        // end location of the macro's last token to get this information.
        auto EndLoc = 0 == MI->getNumTokens() ? MI->getDefinitionEndLoc() :
                                                MI->tokens().back().getEndLoc();
        std::tie(std::ignore, EndDefinitionLocation) =
            tryGetFullSourceLoc(SM, EndLoc);

        JSONPrinter printer{ "Definition" };
        printer.add({
            { "Name", Name },
            { "IsObjectLike", MI->isObjectLike() },
            { "IsDefinitionLocationValid", IsDefinitionLocationValid },
            { "Body", Body },
            { "IsDefinedAtGlobalScope", IsDefinedAtGlobalScope },
            { "DefinitionLocation", DefinitionLocation },
            { "EndDefinitionLocation", EndDefinitionLocation },
        });

        printers.push_back(std::move(printer));
    }

    // Print names of macros inspected by the preprocessor
    for (auto &&Name : DC->InspectedMacroNames) {
        JSONPrinter printer{ "InspectedByCPP" };
        printer.add({ { "Name", Name } });
        printers.push_back(std::move(printer));
    }
    // Print include-directive information
    {
        std::set<llvm::StringRef> LocalIncludes;
        for (auto &&IEL : IC->IncludeEntriesLocs) {
            // Facts for includes
            bool Valid = false;
            std::string IncludeName = "";

            // Check if included at global scope or not
            auto Res = isGlobalInclude(SM, LO, IEL, LocalIncludes, AllDecls);
            if (!Res.first) {
                LocalIncludes.insert(Res.second);
            }

            Valid = Res.first;
            IncludeName = Res.second.empty() ? "" : Res.second.str();

            JSONPrinter printer{ "Include" };
            printer.add(
                { { "IsValid", Valid }, { "IncludeName", IncludeName } });
            printers.push_back(std::move(printer));
        }
    }
    debug("Finished checking includes");

    // Collect certain sets of AST nodes that will be used for checking
    // whether properties are satisfied

    // Any reference to a decl
    std::set<const clang::DeclRefExpr *> AllDeclRefExprs;
    {
        MatchFinder Finder;
        auto Matcher = declRefExpr(unless(anyOf(implicitCastExpr(),
                                                implicitValueInitExpr())))
                           .bind("root");
        StmtCollectorMatchHandler Handler;
        Finder.addMatcher(Matcher, &Handler);
        Finder.matchAST(Ctx);
        for (auto &&ST : Handler.Stmts) {
            AllDeclRefExprs.insert(clang::dyn_cast<clang::DeclRefExpr>(ST));
        }
    }

    // Any reference to a decl declared at a local scope
    // FIXME: Are there more types of decls we should be accounting for?
    // Types, perhaps?
    std::set<const clang::DeclRefExpr *> DeclRefExprsOfLocallyDefinedDecls;
    {
        for (auto &&DRE : AllDeclRefExprs) {
            auto D = DRE->getDecl();
            if (auto VD = clang::dyn_cast<clang::VarDecl>(D)) {
                if (VD->hasLocalStorage()) {
                    DeclRefExprsOfLocallyDefinedDecls.insert(DRE);
                }
            }
        }
    }

    // Any expr with side-effects
    // Binary assignment expressions, Pre/Post Inc/Dec
    std::set<const clang::Expr *> SideEffectExprs;
    {
        MatchFinder Finder;
        auto Matcher =
            expr(allOf(
                     unless(anyOf(implicitCastExpr(), implicitValueInitExpr())),
                     anyOf(binaryOperator(isAssignmentOperator()),
                           unaryOperator(anyOf(hasOperatorName("++"),
                                               hasOperatorName("--"))))))
                .bind("root");
        StmtCollectorMatchHandler Handler;
        Finder.addMatcher(Matcher, &Handler);
        Finder.matchAST(Ctx);
        for (auto &&ST : Handler.Stmts) {
            SideEffectExprs.insert(clang::dyn_cast<clang::Expr>(ST));
        }
    }

    // Any expr that is the modified part of an expression with side-effects
    std::set<clang::Expr *> SideEffectExprsLHSs;
    {
        for (auto &&E : SideEffectExprs) {
            if (auto B = clang::dyn_cast<clang::BinaryOperator>(E)) {
                SideEffectExprsLHSs.insert(B->getLHS());
            } else if (auto U = clang::dyn_cast<clang::UnaryOperator>(E)) {
                SideEffectExprsLHSs.insert(U->getSubExpr());
            }
        }
    }

    // Any expr that is an address-of expr
    std::set<const clang::UnaryOperator *> AddressOfExprs;
    {
        MatchFinder Finder;
        auto Matcher =
            unaryOperator(allOf(unless(anyOf(implicitCastExpr(),
                                             implicitValueInitExpr())),
                                hasOperatorName("&")))
                .bind("root");
        StmtCollectorMatchHandler Handler;
        Finder.addMatcher(Matcher, &Handler);
        Finder.matchAST(Ctx);
        for (auto &&S : Handler.Stmts) {
            AddressOfExprs.insert(clang::dyn_cast<clang::UnaryOperator>(S));
        }
    }

    // Any expr that is an sizeof expr
    std::set<const clang::UnaryExprOrTypeTraitExpr *> SizeOfExprs;
    {
        MatchFinder Finder;
        auto Matcher = sizeOfExpr(unless(anyOf(implicitCastExpr(),
                                               implicitValueInitExpr())))
                           .bind("root");
        StmtCollectorMatchHandler Handler;
        Finder.addMatcher(Matcher, &Handler);
        Finder.matchAST(Ctx);
        for (auto &&S : Handler.Stmts) {
            SizeOfExprs.insert(
                clang::dyn_cast<clang::UnaryExprOrTypeTraitExpr>(S));
        }
    }

    // Any expr that is the operand of an expression with short-circuiting.
    // ConditionalOperator, LogicalAnd, LogicalOr
    std::set<const clang::Expr *> ConditionalExprs;
    {
        MatchFinder Finder;
        auto Matcher =
            expr(allOf(
                     unless(anyOf(implicitCastExpr(), implicitValueInitExpr())),
                     anyOf(conditionalOperator(),
                           binaryOperator(anyOf(hasOperatorName("&&"),
                                                hasOperatorName("||"))))))
                .bind("root");
        StmtCollectorMatchHandler Handler;
        Finder.addMatcher(Matcher, &Handler);
        Finder.matchAST(Ctx);
        for (auto &&ST : Handler.Stmts) {
            if (auto E = clang::dyn_cast<clang::Expr>(ST)) {
                ConditionalExprs.insert(E);
            }
        }
    }

    // Print macro expansion information
    for (auto Exp : MF->Expansions) {
        assert(Exp);
        assert(Exp->MI);

        // String properties
        std::string ASTKind;
        std::string DefinitionLocation;
        std::string InvocationLocation;
        std::string Name;
        std::string TypeSignature;

        // Integer properties
        int InvocationDepth = 0;
        int NumArguments = 0;
        int NumASTRoots = 0;

        // Boolean properties
        bool DoesAnyArgumentContainDeclRefExpr = false;
        bool DoesAnyArgumentHaveSideEffects = false;
        bool DoesBodyContainDeclRefExpr = false;
        bool DoesBodyEndWithCompoundStmt = false;
        bool DoesBodyReferenceDeclDeclaredAfterMacro = false;
        bool DoesBodyReferenceMacroDefinedAfterMacro = false;
        bool DoesSubexpressionExpandedFromBodyHaveLocalType = false;
        bool DoesSubexpressionExpandedFromBodyHaveTypeDefinedAfterMacro = false;
        bool HasAlignedArguments = false;
        bool HasSameNameAsOtherDeclaration = false;
        bool HasStringification = false;
        bool HasTokenPasting = false;
        bool IsAnyArgumentConditionallyEvaluated = false;
        bool IsAnyArgumentExpandedWhereAddressableValueRequired = false;
        bool IsAnyArgumentExpandedInSizeOf = false;
        bool IsAnyArgumentExpandedWhereModifiableValueRequired = false;
        bool IsAnyArgumentExpandedWhereConstExprRequired = false;
        bool IsAnyArgumentNeverExpanded = false;
        bool IsAnyArgumentNotAnExpression = false;
        bool IsAnyArgumentTypeAnonymous = false;
        bool IsAnyArgumentTypeDefinedAfterMacro = false;
        bool IsAnyArgumentTypeLocalType = false;
        bool IsAnyArgumentTypeNull = false;
        bool IsAnyArgumentTypeVoid = false;
        bool IsAnyArgumentTypeFunctionType = false;
        bool IsDefinitionLocationValid = false;
        bool IsExpansionControlFlowStmt = false;
        bool IsExpansionICE = false;
        bool IsExpansionTypeAnonymous = false;
        bool IsExpansionTypeDefinedAfterMacro = false;
        bool IsExpansionTypeLocalType = false;
        bool IsExpansionTypeNull = false;
        bool IsExpansionTypeVoid = false;
        bool IsExpansionTypeFunctionType = false;
        bool IsHygienic = false;
        bool IsICERepresentableByInt16 = false;
        bool IsICERepresentableByInt32 = false;
        bool IsInvocationLocationValid = false;
        bool IsInvokedInMacroArgument = false;
        bool IsInvokedWhereAddressableValueRequired = false;
        bool IsInvokedInSizeOf = false;
        bool IsInvokedWhereICERequired = false;
        bool IsInvokedWhereConstantExpressionRequired = false;
        bool IsInvokedWhereModifiableValueRequired = false;
        bool IsNamePresentInCPPConditional = false;
        bool IsObjectLike = false;

        Name = Exp->Name.str();
        InvocationDepth = Exp->Depth;
        NumArguments = Exp->Arguments.size();
        HasStringification = Exp->HasStringification;
        HasTokenPasting = Exp->HasTokenPasting;

        HasSameNameAsOtherDeclaration =
            // First check if any macro defined before this macro has the
            // same name as any of this macro's parameters
            std::any_of(
                DC->MacroNamesDefinitions.begin(),
                DC->MacroNamesDefinitions.end(),
                [&SM, &Exp](
                    std::pair<std::string, const clang::MacroDirective *> Entry) {
                    return SM.isBeforeInTranslationUnit(
                               SM.getFileLoc(
                                   Entry.second->getDefinition().getLocation()),
                               SM.getFileLoc(Exp->MI->getDefinitionLoc())) &&
                           std::any_of(Exp->Arguments.begin(),
                                       Exp->Arguments.end(),
                                       [&Entry](MacroExpansionArgument Arg) {
                                           return Arg.Name.str() == Entry.first;
                                       });
                }) ||
            // Also check if any global declarations defined before this macro
            // have the same name as this macro
            std::any_of(
                AllDecls.begin(), AllDecls.end(),
                [&SM, &Exp](const clang::Decl *D) {
                    auto ND = clang::dyn_cast_or_null<clang::NamedDecl>(D);
                    if (!ND) {
                        return false;
                    }
                    auto II = ND->getIdentifier();
                    if (!II) {
                        return false;
                    }
                    return II->getName().str() == Exp->Name.str() &&
                           SM.isBeforeInTranslationUnit(
                               SM.getFileLoc(D->getBeginLoc()),
                               SM.getFileLoc(Exp->MI->getDefinitionLoc()));
                });
        IsObjectLike = Exp->MI->isObjectLike();
        IsInvokedInMacroArgument = Exp->InMacroArg;
        IsNamePresentInCPPConditional =
            DC->InspectedMacroNames.find(Exp->Name.str()) !=
            DC->InspectedMacroNames.end();

        // Definition location
        std::tie(IsDefinitionLocationValid, DefinitionLocation) =
            tryGetFullSourceLoc(SM, Exp->MI->getDefinitionLoc());

        // Invocation location
        std::tie(IsInvocationLocationValid, InvocationLocation) =
            tryGetFullSourceLoc(SM, Exp->SpellingRange.getBegin());

        auto DefLoc = SM.getFileLoc(Exp->MI->getDefinitionLoc());

        // Check if any macro this macro invokes were defined after
        // this macro was
        auto Descendants = Exp->getDescendants();

        DoesBodyReferenceMacroDefinedAfterMacro =
            std::any_of(Descendants.begin(), Descendants.end(),
                        [&SM, &Exp](MacroExpansionNode *Desc) {
                            return SM.isBeforeInTranslationUnit(
                                SM.getFileLoc(Exp->MI->getDefinitionLoc()),
                                SM.getFileLoc(Desc->MI->getDefinitionLoc()));
                        });

        // Next get AST information for top level invocations
        if (Exp->Depth == 0 && !Exp->InMacroArg) {
            debug("Top level invocation: ", Exp->Name.str());
            maki::findAlignedASTNodesForExpansion(Exp, Ctx);

            //// Print macro info

            // Exp->dumpMacroInfo(llvm::outs());

            // Exp->dumpASTInfo(llvm::outs(),
            //                  Ctx.getSourceManager(), Ctx.getLangOpts());

            // Number of AST roots
            NumASTRoots = Exp->ASTRoots.size();

            // Determine the AST kind of the expansion
            debug("Checking if expansion has aligned root");
            if (Exp->AlignedRoot) {
                auto D = Exp->AlignedRoot->D;
                auto ST = Exp->AlignedRoot->ST;
                auto TL = Exp->AlignedRoot->TL;

                if (ST) {
                    debug("Aligns with a stmt");
                    ASTKind = "Stmt";
                } else if (D) {
                    debug("Aligns with a decl");
                    ASTKind = "Decl";
                } else if (TL) {
                    debug("Aligns with a type loc");
                    ASTKind = "TypeLoc";
                    // Check that this type specifier list does not include
                    // a typedef that was defined after the macro was defined
                    // debug("Checking if type loc type is null");
                    IsExpansionTypeNull = TL->isNull();

                    // FIXME: For some reason, this function call sometimes
                    // triggers an error. I have tried to debug it the best I
                    // can, but it seems to be due to a problem with Clang.
                    // Until this is fixed, we will not be able to gather full
                    // data on TypeLocs.
                    //
                    // debug("Checking isBaseTypeDefinedAfter");
                    // IsExpansionTypeDefinedAfterMacro = (!TL->isNull()) &&
                    // isBaseTypeDefinedAfter(TL->getTypePtr(), Ctx, DefLoc);
                    // debug("Finished checking isBaseTypeDefinedAfter");
                } else {
                    assert("Aligns with node that is not a Decl/Stmt/TypeLoc");
                }
            }

            // Check that the number of AST nodes aligned with each argument
            // equals the number of times that argument was expanded
            debug("Checking if arguments are all aligned");
            HasAlignedArguments = std::all_of(
                Exp->Arguments.begin(), Exp->Arguments.end(),
                [](MacroExpansionArgument Arg) {
                    return Arg.AlignedRoots.size() == Arg.NumExpansions;
                });
            debug("Done checking if arguments are all aligned");

            std::set<const clang::Stmt *> StmtsExpandedFromArguments;
            // Semantic properties of the macro's arguments
            if (HasAlignedArguments) {
                debug("Collecting argument subtrees");
                for (auto &&Arg : Exp->Arguments) {
                    for (auto &&Root : Arg.AlignedRoots) {
                        auto STs = subtrees(Root.ST);
                        StmtsExpandedFromArguments.insert(STs.begin(),
                                                          STs.end());
                    }
                }
                debug("Done collecting argument subtrees: ",
                      (unsigned int)StmtsExpandedFromArguments.size());

                auto ExpandedFromArgument =
                    [&StmtsExpandedFromArguments](const clang::Stmt *St) {
                        return StmtsExpandedFromArguments.find(St) !=
                               StmtsExpandedFromArguments.end();
                    };

                DoesAnyArgumentHaveSideEffects =
                    std::any_of(SideEffectExprs.begin(), SideEffectExprs.end(),
                                ExpandedFromArgument);

                DoesAnyArgumentContainDeclRefExpr =
                    std::any_of(AllDeclRefExprs.begin(), AllDeclRefExprs.end(),
                                ExpandedFromArgument);

                IsAnyArgumentExpandedWhereModifiableValueRequired = std::any_of(
                    SideEffectExprs.begin(), SideEffectExprs.end(),
                    [&ExpandedFromArgument](const clang::Expr *E) {
                        // Only consider side-effect expressions which were
                        // not expanded from an argument of the same macro
                        if (!ExpandedFromArgument(E)) {
                            clang::Expr *ModifiedPartOfExpr = nullptr;
                            if (auto B =
                                    clang::dyn_cast<clang::BinaryOperator>(E)) {
                                // Side-effecting binary operators, i.e.,
                                // assignment operators.
                                ModifiedPartOfExpr = B->getLHS();
                            } else if (auto U =
                                           clang::dyn_cast<clang::UnaryOperator>(
                                               E)) {
                                // Side-effecting unary operators, i.e.,
                                // increment/decrement operators.
                                ModifiedPartOfExpr = U->getSubExpr();
                            }
                            // Conservatively return true if any part of the
                            // modified expression was expanded from the macro
                            return isInTree(ModifiedPartOfExpr,
                                            ExpandedFromArgument);
                        }
                        return false;
                    });

                IsAnyArgumentExpandedWhereAddressableValueRequired =
                    std::any_of(
                        AddressOfExprs.begin(), AddressOfExprs.end(),
                        [&ExpandedFromArgument](const clang::UnaryOperator *U) {
                            // Only consider address of expressions
                            // which were not expanded from an argument
                            // of the same macro.
                            if (!ExpandedFromArgument(U)) {
                                auto Operand = U->getSubExpr();
                                // Conservatively return true if any
                                // part of the addressed expression was
                                // expanded from the argument.
                                return isInTree(Operand, ExpandedFromArgument);
                            }
                            return false;
                        });

                IsAnyArgumentExpandedInSizeOf = std::any_of(
                    SizeOfExprs.begin(), SizeOfExprs.end(),
                    [&ExpandedFromArgument](
                        const clang::UnaryExprOrTypeTraitExpr *U) {
                        // Only consider sizeof expressions which were not
                        // expanded from an argument of the same macro.
                        if (!U->isArgumentType() && !ExpandedFromArgument(U)) {
                            auto Operand = U->getArgumentExpr();
                            // Conservatively return true if any
                            // part of the size of'd expression was
                            // expanded from the argument.
                            return isInTree(Operand, ExpandedFromArgument);
                        }
                        return false;
                    });
            }

            std::set<const clang::Stmt *> StmtsExpandedFromBody;
            // Semantic properties of the macro body
            if (Exp->AlignedRoot && Exp->AlignedRoot->ST &&
                HasAlignedArguments) {
                auto ST = Exp->AlignedRoot->ST;

                debug("Collecting body subtrees");
                StmtsExpandedFromBody = subtrees(ST);
                // Remove all Stmts which were actually expanded from arguments
                for (auto &&St : StmtsExpandedFromArguments) {
                    StmtsExpandedFromBody.erase(St);
                }

                auto ExpandedFromBody =
                    [&StmtsExpandedFromBody](const clang::Stmt *St) {
                        return StmtsExpandedFromBody.find(St) !=
                               StmtsExpandedFromBody.end();
                    };

                debug("Checking if any argument is conditionally "
                      "evaluated in the body of the expansion");
                IsAnyArgumentConditionallyEvaluated = std::any_of(
                    ConditionalExprs.begin(), ConditionalExprs.end(),
                    [&ExpandedFromBody,
                     &StmtsExpandedFromArguments](const clang::Expr *CE) {
                        return ExpandedFromBody(CE) &&
                               std::any_of(StmtsExpandedFromArguments.begin(),
                                           StmtsExpandedFromArguments.end(),
                                           [&CE](const clang::Stmt *ArgStmt) {
                                               return inTree(ArgStmt, CE);
                                           });
                    });
                debug("Done checking if any argument is conditionally "
                      "evaluated in the body of the expansion");

                // NOTE: This may not be correct if the definition of
                // of the decl is separate from its declaration.
                DoesBodyReferenceDeclDeclaredAfterMacro = std::any_of(
                    AllDeclRefExprs.begin(), AllDeclRefExprs.end(),
                    [&SM, &DefLoc,
                     &ExpandedFromBody](const clang::DeclRefExpr *DRE) {
                        if (ExpandedFromBody(DRE)) {
                            auto D = DRE->getDecl();
                            auto DeclLoc = SM.getFileLoc(D->getLocation());

                            return SM.isBeforeInTranslationUnit(DefLoc,
                                                                DeclLoc);
                        }
                        return false;
                    });

                DoesBodyContainDeclRefExpr =
                    std::any_of(AllDeclRefExprs.begin(), AllDeclRefExprs.end(),
                                ExpandedFromBody);

                DoesBodyEndWithCompoundStmt = endsWithCompound(ST);

                DoesSubexpressionExpandedFromBodyHaveLocalType = std::any_of(
                    StmtsExpandedFromBody.begin(), StmtsExpandedFromBody.end(),
                    [&Ctx](const clang::Stmt *St) {
                        // If this is a sizeof/cast expression, check if the
                        // type passed to the expression was defined after this
                        // macro.
                        if (auto CastOrSizeOf =
                                clang::dyn_cast<clang::UnaryExprOrTypeTraitExpr>(
                                    St)) {
                            auto T = CastOrSizeOf->getTypeOfArgument()
                                         .getTypePtrOrNull();
                            return isBaseTypeLocal(T);
                        }
                        if (auto E = clang::dyn_cast<clang::Expr>(St)) {
                            // Also check if the type of this subexpression was
                            // defined after this macro.
                            auto T = E->getType().getTypePtrOrNull();
                            return isBaseTypeLocal(T);
                        }
                        return false;
                    });

                DoesSubexpressionExpandedFromBodyHaveTypeDefinedAfterMacro =
                    std::any_of(
                        StmtsExpandedFromBody.begin(),
                        StmtsExpandedFromBody.end(),
                        [&Ctx, &DefLoc](const clang::Stmt *St) {
                            if (auto SizeOfOrAlignOf = clang::dyn_cast<
                                    clang::UnaryExprOrTypeTraitExpr>(St)) {
                                // If this is a sizeof/alignof expression, check
                                // if the type passed to the expression was
                                // defined after this macro. We can't ignore
                                // typedefs in this case because the name of the
                                // typedef'd type will be in the expression
                                // itself.
                                auto T = SizeOfOrAlignOf->getTypeOfArgument()
                                             .getTypePtrOrNull();
                                return isAnyDesugaredTypeDefinedAfter(T, Ctx,
                                                                      DefLoc);
                            } else if (auto Cast = clang::dyn_cast<
                                           clang::ExplicitCastExpr>(St)) {
                                // Ditto for cast expressions.
                                auto T =
                                    Cast->getTypeAsWritten().getTypePtrOrNull();
                                return isAnyDesugaredTypeDefinedAfter(T, Ctx,
                                                                      DefLoc);

                            } else if (auto CL = clang::dyn_cast<
                                           clang::CompoundLiteralExpr>(St)) {
                                // Ditto for compound literal expressions.
                                auto T = CL->getType().getTypePtrOrNull();
                                return isAnyDesugaredTypeDefinedAfter(T, Ctx,
                                                                      DefLoc);
                            } else if (auto E =
                                           clang::dyn_cast<clang::Expr>(St)) {
                                auto T = E->getType().getTypePtrOrNull();
                                return
                                    // Ditto for typeof types.
                                    [&T, &Ctx, &DefLoc]() {
                                        if (auto ToT = clang::dyn_cast_or_null<
                                                clang::TypeOfType>(T)) {
                                            auto Ty = ToT->getUnmodifiedType()
                                                          .getTypePtrOrNull();
                                            return isAnyDesugaredTypeDefinedAfter(
                                                Ty, Ctx, DefLoc);
                                        }
                                        return false;
                                    }() ||
                                    // Also check if the type of this
                                    // subexpression was defined after this
                                    // macro. We can ignore typedefs in this
                                    // case because expressions evaluating to
                                    // typedef'd types are valid so long as the
                                    // underlying type is defined before the
                                    // expression is evaluated.
                                    isBaseTypeDefinedAfter(T, Ctx, DefLoc);
                            }
                            return false;
                        });

                // We only allow references to declarations declared
                // within the macro expansion itself
                IsHygienic = std::none_of(
                    DeclRefExprsOfLocallyDefinedDecls.begin(),
                    DeclRefExprsOfLocallyDefinedDecls.end(),
                    [&ST, &SM,
                     &ExpandedFromBody](const clang::DeclRefExpr *DRE) {
                        // References that don't come from the macro's body
                        // are fine
                        if (!ExpandedFromBody(DRE)) {
                            return false;
                        }

                        auto B = SM.getFileLoc(ST->getBeginLoc());
                        auto E = SM.getFileLoc(ST->getEndLoc());
                        auto D = DRE->getDecl();
                        if (!D) {
                            return false;
                        }

                        auto L = SM.getFileLoc(D->getLocation());
                        // NOTE: It would be nice if we could instead walk
                        // the AST and check if this decl is under the AST
                        // aligned with this macro.
                        // This should work for now though.
                        return !clang::SourceRange(B, E).fullyContains(L);
                    });

                IsInvokedWhereModifiableValueRequired = std::any_of(
                    SideEffectExprs.begin(), SideEffectExprs.end(),
                    [&ST, &ExpandedFromBody](const clang::Expr *E) {
                        // Only consider side-effect expressions which were
                        // not expanded from the body of the same macro
                        if (!ExpandedFromBody(E)) {
                            clang::Expr *LHS = nullptr;
                            auto B = clang::dyn_cast<clang::BinaryOperator>(E);
                            auto U = clang::dyn_cast<clang::UnaryOperator>(E);
                            if (B) {
                                LHS = B->getLHS();
                            } else if (U) {
                                LHS = U->getSubExpr();
                            }
                            return inTree(ST, LHS);
                        }
                        return false;
                    });

                IsInvokedWhereAddressableValueRequired = std::any_of(
                    AddressOfExprs.begin(), AddressOfExprs.end(),
                    [&ST, &ExpandedFromBody](const clang::UnaryOperator *U) {
                        // Only consider address of expressions which were
                        // not expanded from the body of the same macro
                        if (!ExpandedFromBody(U)) {
                            auto Operand = U->getSubExpr();
                            Operand = Operand->IgnoreImplicit();
                            return inTree(ST, Operand);
                        }
                        return false;
                    });

                IsInvokedInSizeOf = std::any_of(
                    SizeOfExprs.begin(), SizeOfExprs.end(),
                    [&ST, &ExpandedFromBody](
                        const clang::UnaryExprOrTypeTraitExpr *U) {
                        // Only consider size of expressions which
                        // were not expanded from the body of the
                        // same macro
                        if (!U->isArgumentType() && !ExpandedFromBody(U)) {
                            auto Operand = U->getArgumentExpr();
                            Operand = Operand->IgnoreImplicit();
                            return inTree(ST, Operand);
                        }
                        return false;
                    });

                IsInvokedWhereICERequired =
                    isDescendantOfNodeRequiringICE(Ctx, ST);
                IsInvokedWhereConstantExpressionRequired =
                    IsInvokedWhereICERequired |
                    isDescendantOfNodeRequiringConstantExpression(Ctx, ST);

                //// Generate type signature

                // Body type information
                TypeSignature = "void";
                if (auto E = clang::dyn_cast<clang::Expr>(ST)) {
                    ASTKind = "Expr";

                    // Type information about the entire expansion
                    auto QT = E->getType();
                    auto T = QT.getTypePtrOrNull();
                    IsExpansionTypeNull = QT.isNull() || T == nullptr;

                    if (T) {
                        IsExpansionTypeVoid = T->isVoidType();
                        IsExpansionTypeFunctionType =
                            T->isFunctionType() || T->isFunctionPointerType();
                        IsExpansionTypeAnonymous = isBaseTypeAnonymous(T);
                        IsExpansionTypeLocalType = isBaseTypeLocal(T);
                        auto CT = QT.getDesugaredType(Ctx)
                                      .getUnqualifiedType()
                                      .getCanonicalType();

                        // If the expansion is an array, decay to pointer
                        if (CT->isArrayType()) {
                            CT = Ctx.getArrayDecayedType(CT);
                        }

                        TypeSignature = CT.getAsString();
                    }
                    // We can ignore typedef types in the macro's return type
                    // since we emit the underlying type in the translated type
                    // signature, which is interhchangeable with the typedef
                    // type.
                    IsExpansionTypeDefinedAfterMacro = isBaseTypeDefinedAfter(
                        QT.getTypePtrOrNull(), Ctx, DefLoc);

                    // Properties of integral constant expressions
                    if (auto ICE = E->getIntegerConstantExpr(Ctx)) {
                        IsExpansionICE = true;
                        // These checks are based on the following LLVM function
                        // definition:
                        // https://llvm.org/doxygen/APSInt_8h_source.html#l00089
                        if (ICE->isNegative()) {
                            IsICERepresentableByInt16 = ICE->isSignedIntN(16);
                            IsICERepresentableByInt32 = ICE->isSignedIntN(32);
                        } else {
                            // Treat positive values as unsigned to maximize the
                            // available number of bits.
                            IsICERepresentableByInt16 = ICE->isIntN(16);
                            IsICERepresentableByInt32 = ICE->isIntN(32);
                        }
                    }
                }
                // Macro identifier
                TypeSignature += " " + Name;

                // Argument type information
                IsAnyArgumentNotAnExpression = false;
                IsAnyArgumentTypeNull = false;
                IsAnyArgumentTypeDefinedAfterMacro = false;

                if (Exp->MI->isFunctionLike() &&
                    (ASTKind == "Stmt" || ASTKind == "Expr")) {
                    TypeSignature += "(";
                }
                debug("Iterating arguments");
                int ArgNum = 0;
                for (auto &&Arg : Exp->Arguments) {
                    if (ArgNum != 0) {
                        TypeSignature += ", ";
                    }
                    ArgNum += 1;

                    IsAnyArgumentNeverExpanded |= Arg.AlignedRoots.empty();

                    if (Arg.AlignedRoots.empty()) {
                        continue;
                    }

                    auto Arg1stExpST = Arg.AlignedRoots.front().ST;
                    auto E = clang::dyn_cast_or_null<clang::Expr>(Arg1stExpST);

                    IsAnyArgumentNotAnExpression |= (E == nullptr);

                    debug("Checking if argument is an expression");

                    if (!E) {
                        continue;
                    }

                    IsAnyArgumentExpandedWhereConstExprRequired |=
                        isDescendantOfNodeRequiringICE(Ctx, E) ||
                        isDescendantOfNodeRequiringConstantExpression(Ctx, E);

                    std::string ArgTypeStr = "<Null>";

                    // Type information about arguments
                    auto QT = E->getType();
                    auto T = QT.getTypePtrOrNull();
                    IsAnyArgumentTypeNull |= QT.isNull() || T == nullptr;

                    if (T) {
                        IsAnyArgumentTypeVoid |= T->isVoidType();
                        IsAnyArgumentTypeFunctionType |=
                            T->isFunctionType() || T->isFunctionPointerType();
                        IsAnyArgumentTypeAnonymous |= isBaseTypeAnonymous(T);
                        IsAnyArgumentTypeLocalType |= isBaseTypeLocal(T);
                        auto CT = QT.getDesugaredType(Ctx)
                                      .getUnqualifiedType()
                                      .getCanonicalType();

                        // If the argument is an array, decay to pointer
                        if (CT->isArrayType()) {
                            CT = Ctx.getArrayDecayedType(CT);
                        }

                        ArgTypeStr = CT.getAsString();
                    }
                    // We can ignore typedef types in the macro's argument types
                    // since we emit the underlying type for each argument in
                    // the translated type signature, and a typedef and its
                    // underlying type are interhchangeable.
                    IsAnyArgumentTypeDefinedAfterMacro |=
                        isBaseTypeDefinedAfter(QT.getTypePtrOrNull(), Ctx,
                                               DefLoc);

                    TypeSignature += ArgTypeStr;
                    TypeSignature += " " + Arg.Name.str();
                }
                debug("Finished iterating arguments");
                if (Exp->MI->isFunctionLike() &&
                    (ASTKind == "Stmt" || ASTKind == "Expr")) {
                    TypeSignature += ")";
                }
            }

            // Set of all Stmts expanded from macro
            std::set<const clang::Stmt *> AllStmtsExpandedFromMacro =
                StmtsExpandedFromBody;
            AllStmtsExpandedFromMacro.insert(StmtsExpandedFromArguments.begin(),
                                             StmtsExpandedFromArguments.end());

            IsExpansionControlFlowStmt = std::any_of(
                AllStmtsExpandedFromMacro.begin(),
                AllStmtsExpandedFromMacro.end(), [](const clang::Stmt *St) {
                    return llvm::isa_and_nonnull<clang::ReturnStmt>(St) ||
                           llvm::isa_and_nonnull<clang::ContinueStmt>(St) ||
                           llvm::isa_and_nonnull<clang::BreakStmt>(St) ||
                           llvm::isa_and_nonnull<clang::GotoStmt>(St);
                });
        }

        JSONPrinter printer{ "Invocation" };

        printer.add({
            { "Name", Name },
            { "DefinitionLocation", DefinitionLocation },
            { "InvocationLocation", InvocationLocation },
            { "ASTKind", ASTKind },
            { "TypeSignature", TypeSignature },
        });

        printer.add({
            { "InvocationDepth", InvocationDepth },
            { "NumASTRoots", NumASTRoots },
            { "NumArguments", NumArguments },
        });

        printer.add(
            { { "HasStringification", HasStringification },
              { "HasTokenPasting", HasTokenPasting },
              { "HasAlignedArguments", HasAlignedArguments },
              { "HasSameNameAsOtherDeclaration",
                HasSameNameAsOtherDeclaration },

              { "IsExpansionControlFlowStmt", IsExpansionControlFlowStmt },

              { "DoesBodyReferenceMacroDefinedAfterMacro",
                DoesBodyReferenceMacroDefinedAfterMacro },
              { "DoesBodyReferenceDeclDeclaredAfterMacro",
                DoesBodyReferenceDeclDeclaredAfterMacro },
              { "DoesBodyContainDeclRefExpr", DoesBodyContainDeclRefExpr },
              { "DoesBodyEndWithCompoundStmt", DoesBodyEndWithCompoundStmt },
              { "DoesSubexpressionExpandedFromBodyHaveLocalType",
                DoesSubexpressionExpandedFromBodyHaveLocalType },
              { "DoesSubexpressionExpandedFromBodyHaveTypeDefinedAfterMacro",
                DoesSubexpressionExpandedFromBodyHaveTypeDefinedAfterMacro },

              { "DoesAnyArgumentHaveSideEffects",
                DoesAnyArgumentHaveSideEffects },
              { "DoesAnyArgumentContainDeclRefExpr",
                DoesAnyArgumentContainDeclRefExpr },

              { "IsHygienic", IsHygienic },
              { "IsICERepresentableByInt16", IsICERepresentableByInt16 },
              { "IsICERepresentableByInt32", IsICERepresentableByInt32 },
              { "IsDefinitionLocationValid", IsDefinitionLocationValid },
              { "IsInvocationLocationValid", IsInvocationLocationValid },
              { "IsObjectLike", IsObjectLike },
              { "IsInvokedInMacroArgument", IsInvokedInMacroArgument },
              { "IsNamePresentInCPPConditional",
                IsNamePresentInCPPConditional },
              { "IsExpansionICE", IsExpansionICE },

              { "IsExpansionTypeNull", IsExpansionTypeNull },
              { "IsExpansionTypeAnonymous", IsExpansionTypeAnonymous },
              { "IsExpansionTypeLocalType", IsExpansionTypeLocalType },
              { "IsExpansionTypeDefinedAfterMacro",
                IsExpansionTypeDefinedAfterMacro },
              { "IsExpansionTypeVoid", IsExpansionTypeVoid },
              { "IsExpansionTypeFunctionType", IsExpansionTypeFunctionType },

              { "IsAnyArgumentTypeNull", IsAnyArgumentTypeNull },
              { "IsAnyArgumentTypeAnonymous", IsAnyArgumentTypeAnonymous },
              { "IsAnyArgumentTypeLocalType", IsAnyArgumentTypeLocalType },
              { "IsAnyArgumentTypeDefinedAfterMacro",
                IsAnyArgumentTypeDefinedAfterMacro },
              { "IsAnyArgumentTypeVoid", IsAnyArgumentTypeVoid },
              { "IsAnyArgumentTypeFunctionType",
                IsAnyArgumentTypeFunctionType },

              { "IsInvokedWhereModifiableValueRequired",
                IsInvokedWhereModifiableValueRequired },
              { "IsInvokedWhereAddressableValueRequired",
                IsInvokedWhereAddressableValueRequired },
              { "IsInvokedInSizeOf", IsInvokedInSizeOf },
              { "IsInvokedWhereICERequired", IsInvokedWhereICERequired },
              { "IsInvokedWhereConstantExpressionRequired",
                IsInvokedWhereConstantExpressionRequired },

              { "IsAnyArgumentExpandedWhereModifiableValueRequired",
                IsAnyArgumentExpandedWhereModifiableValueRequired },
              { "IsAnyArgumentExpandedWhereAddressableValueRequired",
                IsAnyArgumentExpandedWhereAddressableValueRequired },
              { "IsAnyArgumentExpandedInSizeOf",
                IsAnyArgumentExpandedInSizeOf },
              { "IsAnyArgumentConditionallyEvaluated",
                IsAnyArgumentConditionallyEvaluated },
              { "IsAnyArgumentExpandedWhereConstExprRequired",
                IsAnyArgumentExpandedWhereConstExprRequired },
              { "IsAnyArgumentNeverExpanded", IsAnyArgumentNeverExpanded },
              { "IsAnyArgumentNotAnExpression",
                IsAnyArgumentNotAnExpression } });

        printers.push_back(std::move(printer));
    }

    maki::JSONPrinter::printJSONArray(printers);

    // Only delete top level expansions since deconstructor deletes nested
    // expansions
    for (auto &&Exp : MF->Expansions) {
        if (Exp->Depth == 0) {
            delete Exp;
        }
    }
}
} // namespace maki
