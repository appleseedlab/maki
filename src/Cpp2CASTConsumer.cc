#include "Cpp2CASTConsumer.hh"
#include "ASTUtils.hh"
#include "DeclStmtTypeLoc.hh"
#include "DeclCollectorMatchHandler.hh"
#include "ExpansionMatchHandler.hh"
#include "AlignmentMatchers.hh"
#include "IncludeCollector.hh"
#include "Logging.hh"
#include "StmtCollectorMatchHandler.hh"

#include "clang/Lex/Lexer.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

#include <algorithm>
#include <functional>
#include <set>
#include <queue>

#include "assert.h"

// NOTE:    We can't use TK_IgnoreUnlessSpelledInSource because it ignores
//          paren exprs

namespace cpp2c
{
    using namespace clang::ast_matchers;

    // Collect all subtrees of the given stmt using BFS
    std::set<const clang::Stmt *> subtrees(const clang::Stmt *ST)
    {
        std::set<const clang::Stmt *> Subtrees;
        if (!ST)
            return Subtrees;

        std::queue<const clang::Stmt *> Q({ST});
        while (!Q.empty())
        {
            auto Cur = Q.front();
            Q.pop();
            if (Cur)
            {
                Subtrees.insert(Cur);
                for (auto &&Child : Cur->children())
                    Q.push(Child);
            }
        }
        return Subtrees;
    }

    clang::Expr *skipImplicitAndParens(clang::Expr *E)
    {
        while (E && (llvm::isa_and_nonnull<clang::ParenExpr>(E) ||
                     llvm::isa_and_nonnull<clang::ImplicitCastExpr>(E)))
            if (auto P = clang::dyn_cast<clang::ParenExpr>(E))
                E = P->getSubExpr();
            else if (auto I = clang::dyn_cast<clang::ImplicitCastExpr>(E))
                E = I->getSubExpr();
        return E;
    }

    // Returns true if LHS is a subtree of RHS via BFS
    bool inTree(const clang::Stmt *LHS, const clang::Stmt *RHS)
    {
        std::queue<const clang::Stmt *> Q({RHS});
        while (!Q.empty())
        {
            auto Cur = Q.front();
            Q.pop();
            if (LHS == Cur)
                return true;
            if (Cur)
                for (auto &&Child : Cur->children())
                    Q.push(Child);
        }
        return false;
    }

    // Returns true if the given predicate returns true for any type
    // contained in the given type
    bool isInType(
        const clang::Type *T,
        clang::ASTContext &Ctx,
        std::function<bool(const clang::Type *)> pred)
    {
        debug("Checking if T is a pointer type");
        while (T && (T->isAnyPointerType() || T->isArrayType()))
            if (T->isAnyPointerType())
                T = T->getPointeeType().getTypePtrOrNull();
            else if (T->isArrayType())
                T = T->getBaseElementTypeUnsafe();

        return pred(T);
    }

    clang::Decl *getTypeDeclOrNull(const clang::Type *T)
    {
        if (!T)
            return nullptr;

        if (auto TD = clang::dyn_cast<clang::TypedefType>(T))
            return TD->getDecl();
        else if (auto TD = clang::dyn_cast<clang::TagType>(T))
            return TD->getDecl();
        else if (auto ET = clang::dyn_cast<clang::ElaboratedType>(T))
            return getTypeDeclOrNull(ET->desugar().getTypePtrOrNull());
        else
            return nullptr;
    }

    // Returns true if any type in T was defined after L
    bool hasTypeDefinedAfter(
        const clang::Type *T,
        clang::ASTContext &Ctx,
        clang::SourceLocation L)
    {
        auto &SM = Ctx.getSourceManager();
        return isInType(
            T,
            Ctx,
            [&SM, L](const clang::Type *T)
            {
                if (!T)
                    return false;

                auto *D = getTypeDeclOrNull(T);

                if (!D)
                    return false;

                auto DLoc = D->getLocation();
                if (DLoc.isInvalid())
                    return false;

                auto DFLoc = SM.getFileLoc(DLoc);
                if (DFLoc.isInvalid())
                    return false;

                return SM.isBeforeInTranslationUnit(L, DFLoc);
            });
    }

    // Returns true if any type in T is an anonymous type
    bool hasAnonymousType(const clang::Type *T, clang::ASTContext &Ctx)
    {
        return isInType(
            T,
            Ctx,
            [](const clang::Type *T)
            {
                if (!T)
                    return false;

                auto D = getTypeDeclOrNull(T);
                if (!D)
                    return false;

                auto ND = clang::dyn_cast<clang::NamedDecl>(D);
                if (!ND)
                    return false;

                return ND->getIdentifier() == nullptr || ND->getName().empty();
            });
    }

    // Returns true if any type in T is a local type
    bool hasLocalType(const clang::Type *T, clang::ASTContext &Ctx)
    {
        return isInType(
            T,
            Ctx,
            [](const clang::Type *T)
            {
                if (!T)
                    return false;

                auto D = getTypeDeclOrNull(T);
                if (!D)
                    return false;

                auto DCtx = D->getDeclContext();

                return !DCtx->isTranslationUnit();
            });
    }

    // Returns true if ST is a descendant of a Stmt which can only have
    // subexpressions that are integral constants expressions
    bool isDescendantOfStmtRequiringICE(clang::ASTContext &Ctx,
                                        const clang::Stmt *ST)
    {
        if (!ST)
            return false;

        std::queue<clang::DynTypedNode> Q;
        for (auto P : Ctx.getParents(*ST))
            Q.push(P);
        while (!Q.empty())
        {
            auto Cur = Q.front();
            Q.pop();

            if (Cur.get<clang::CaseStmt>() ||
                Cur.get<clang::EnumDecl>())
                return true;

            if (auto FD = Cur.get<clang::FieldDecl>())
                if (FD->isBitField())
                    return true;

            if (auto VD = Cur.get<clang::VarDecl>())
            {
                auto QT = VD->getType();
                if (!QT.isNull())
                {
                    if (auto T = QT.getTypePtrOrNull())
                        if (T->isArrayType())
                            return true;
                }
            }

            for (auto P : Ctx.getParents(Cur))
                Q.push(P);
        }
        return false;
    }

    // Tries to get the full real path and line + column number for a given
    // source location.
    // First element is whether the operation was successful, the second
    // is the error if not and the full path if successful.
    std::pair<bool, std::string> tryGetFullSourceLoc(
        clang::SourceManager &SM,
        clang::SourceLocation L)
    {
        if (L.isValid())
        {
            auto FID = SM.getFileID(L);
            if (FID.isValid())
            {
                if (auto FE = SM.getFileEntryForID(FID))
                {
                    auto Name = FE->tryGetRealPathName();
                    if (!Name.empty())
                    {
                        auto FLoc = SM.getFileLoc(L);
                        if (FLoc.isValid())
                        {
                            auto s = FLoc.printToString(SM);
                            // Find second-to-last colon
                            auto i = s.rfind(':', s.rfind(':') - 1);
                            return {true, Name.str() + ":" + s.substr(i + 1)};
                        }
                        return {false, "Invalid File SLoc"};
                    }
                    return {false, "Nameless file"};
                }
                return {false, "File without FileEntry"};
            }
            return {false, "Invalid file ID"};
        }
        return {false, "Invalid SLoc"};
    }

    // Checks if the included file is a globally included file.
    // The first element of the return result if false if not;
    // true otherwise.
    // The second element is the name of the included file.
    std::pair<bool, llvm::StringRef> isGlobalInclude(
        clang::SourceManager &SM,
        const clang::LangOptions &LO,
        std::pair<clang::OptionalFileEntryRef, clang::SourceLocation> &IEL,
        std::set<llvm::StringRef> &LocalIncludes,
        std::vector<const clang::Decl *> &Decls)
    {
        auto FE = IEL.first;
        auto HashLoc = IEL.second;

        // Check that the included file has a value
        if (!FE.has_value())
            return {false, "<null>"};

        // Check that the included file actually has a name
        auto IncludedFileRealpath = FE->getFileEntry().tryGetRealPathName();
        if (IncludedFileRealpath.empty())
            return {false, IncludedFileRealpath};

        // Check that the hash location is valid
        if (HashLoc.isInvalid())
            return {false, IncludedFileRealpath};

        // Check that the file the file is included in is valid
        auto IncludedInFID = SM.getFileID(HashLoc);
        if (IncludedInFID.isInvalid())
            return {false, IncludedFileRealpath};

        // Check that a file entry exists for the  file the file is included in
        auto IncludedInFE = SM.getFileEntryForID(IncludedInFID);
        if (!IncludedInFE)
            return {false, IncludedFileRealpath};

        // Check that a real path exists for the file the file is included in
        auto IncludedInRealpath = IncludedInFE->tryGetRealPathName();
        if (IncludedInRealpath.empty())
            return {false, IncludedFileRealpath};

        // Check that the file the file is included in is not in turn included
        // in a file included in a non-global scope
        if (LocalIncludes.find(IncludedInRealpath) != LocalIncludes.end())
            return {false, IncludedFileRealpath};

        auto HashFLoc = SM.getFileLoc(HashLoc);
        // Check that the file location is valid
        if (HashFLoc.isInvalid())
            return {false, IncludedFileRealpath};

        // Check that the include does not appear within the range of any
        // declaration in the file
        if (std::any_of(
                Decls.begin(),
                Decls.end(),
                [&SM, &LO, &HashFLoc](const clang::Decl *D)
                {
                    auto B = SM.getFileLoc(D->getBeginLoc());
                    auto E = SM.getFileLoc(D->getEndLoc());

                    if (B.isInvalid() || E.isInvalid())
                        return false;

                    // Include the location just after the declaration
                    // to account for semicolons.
                    // If the decl does not have semicolon after it,
                    // that's fine since it would be a non-global
                    // location anyway
                    if (auto Tok = clang::Lexer::findNextToken(E, SM, LO))
                        if (Tok.has_value())
                            E = SM.getFileLoc(Tok.value().getEndLoc());

                    if (E.isInvalid())
                        return false;

                    return clang::SourceRange(B, E).fullyContains(HashFLoc);
                }))
            return {false, IncludedFileRealpath};

        // Success
        return {true, IncludedFileRealpath};
    }

    Cpp2CASTConsumer::Cpp2CASTConsumer(clang::CompilerInstance &CI)
    {
        clang::Preprocessor &PP = CI.getPreprocessor();
        clang::ASTContext &Ctx = CI.getASTContext();

        MF = new cpp2c::MacroForest(PP, Ctx);
        IC = new cpp2c::IncludeCollector();
        DC = new cpp2c::DefinitionInfoCollector(Ctx);

        PP.addPPCallbacks(std::unique_ptr<cpp2c::MacroForest>(MF));
        PP.addPPCallbacks(std::unique_ptr<cpp2c::IncludeCollector>(IC));
        PP.addPPCallbacks(std::unique_ptr<cpp2c::DefinitionInfoCollector>(DC));
    }

    void Cpp2CASTConsumer::HandleTranslationUnit(clang::ASTContext &Ctx)
    {
        auto &SM = Ctx.getSourceManager();
        auto &LO = Ctx.getLangOpts();

        // Print definition information
        for (auto &&Entry : DC->MacroNamesDefinitions)
        {
            std::string Name = Entry.first,
                        DefLocOrError;
            bool Valid;

            auto MD = Entry.second;
            auto DefLoc = MD ? SM.getFileLoc(MD->getDefinition().getLocation())
                             : clang::SourceLocation();
            Valid = DefLoc.isValid();

            // Try to get the full path to the DefLoc
            auto Res = tryGetFullSourceLoc(SM, DefLoc);
            Valid &= Res.first;
            DefLocOrError = Res.second;

            auto MI = MD->getMacroInfo();
            assert(MI);

            print("Definition", Name, MI->isObjectLike(), Valid, DefLocOrError);
        }

        // Collect declaration ranges
        std::vector<const clang::Decl *> TopLevelDecls =
            ({
                MatchFinder Finder;
                DeclCollectorMatchHandler Handler;
                auto Matcher = decl(unless(anyOf(
                                        isImplicit(),
                                        translationUnitDecl())))
                                   .bind("root");
                Finder.addMatcher(Matcher, &Handler);
                Finder.matchAST(Ctx);
                Handler.Decls;
            });

        // Print names of macros inspected by the preprocessor
        for (auto &&Name : DC->InspectedMacroNames)
            print("InspectedByCPP", Name);
        // Print include-directive information
        {
            std::set<llvm::StringRef> LocalIncludes;
            for (auto &&IEL : IC->IncludeEntriesLocs)
            {
                // Facts for includes
                bool Valid = false;
                std::string IncludeName = "";

                // Check if included at global scope or not
                auto Res = isGlobalInclude(SM, LO, IEL, LocalIncludes,
                                           TopLevelDecls);
                if (!Res.first)
                    LocalIncludes.insert(Res.second);

                Valid = Res.first;
                IncludeName = Res.second.empty() ? "" : Res.second.str();

                print("Include", Valid, IncludeName);
            }
        }
        debug("Finished checking includes");


        // Collect certain sets of AST nodes that will be used for checking
        // whether properties are satisfied

        // Any reference to a decl
        std::set<const clang::DeclRefExpr *> AllDeclRefExprs;
        {
            MatchFinder Finder;
            auto Matcher = declRefExpr(
                               unless(
                                   anyOf(
                                       implicitCastExpr(),
                                       implicitValueInitExpr())))
                               .bind("root");
            StmtCollectorMatchHandler Handler;
            Finder.addMatcher(Matcher, &Handler);
            Finder.matchAST(Ctx);
            for (auto &&ST : Handler.Stmts)
                AllDeclRefExprs.insert(clang::dyn_cast<clang::DeclRefExpr>(ST));
        }

        // Any reference to a decl declared at a local scope
        // FIXME: Are there more types of decls we should be accounting for?
        // Types, perhaps?
        std::set<const clang::DeclRefExpr *> DeclRefExprsOfLocallyDefinedDecls;
        {
            for (auto &&DRE : AllDeclRefExprs)
            {
                auto D = DRE->getDecl();
                if (auto VD = clang::dyn_cast<clang::VarDecl>(D))
                    if (VD->hasLocalStorage())
                        DeclRefExprsOfLocallyDefinedDecls.insert(DRE);
            }
        }

        // Any expr with side-effects
        // Binary assignment expressions, Pre/Post Inc/Dec
        std::set<const clang::Expr *> SideEffectExprs;
        {
            MatchFinder Finder;
            auto Matcher = expr(
                               allOf(
                                   unless(
                                       anyOf(
                                           implicitCastExpr(),
                                           implicitValueInitExpr())),
                                   anyOf(
                                       binaryOperator(
                                           isAssignmentOperator()),
                                       unaryOperator(
                                           anyOf(
                                               hasOperatorName("++"),
                                               hasOperatorName("--"))))))
                               .bind("root");
            StmtCollectorMatchHandler Handler;
            Finder.addMatcher(Matcher, &Handler);
            Finder.matchAST(Ctx);
            for (auto &&ST : Handler.Stmts)
                SideEffectExprs.insert(clang::dyn_cast<clang::Expr>(ST));
        }

        // Any expr that is the modified part of an expression with side-effects
        std::set<clang::Expr *> SideEffectExprsLHSs;
        {
            for (auto &&E : SideEffectExprs)
            {
                if (auto B = clang::dyn_cast<clang::BinaryOperator>(E))
                    SideEffectExprsLHSs.insert(B->getLHS());
                else if (auto U = clang::dyn_cast<clang::UnaryOperator>(E))
                    SideEffectExprsLHSs.insert(U->getSubExpr());
            }
        }

        // Any expr that is an address-of expr
        std::set<const clang::UnaryOperator *> AddressOfExprs;
        {
            MatchFinder Finder;
            auto Matcher = unaryOperator(
                               allOf(
                                   unless(
                                       anyOf(
                                           implicitCastExpr(),
                                           implicitValueInitExpr())),
                                   hasOperatorName("&")))
                               .bind("root");
            StmtCollectorMatchHandler Handler;
            Finder.addMatcher(Matcher, &Handler);
            Finder.matchAST(Ctx);
            for (auto &&S : Handler.Stmts)
                AddressOfExprs.insert(clang::dyn_cast<clang::UnaryOperator>(S));
        }

        // Any expr that is the operand of an expression with short-circuiting.
        // ConditionalOperator, LogicalAnd, LogicalOr
        std::set<const clang::Expr *> ConditionalExprs;
        {
            MatchFinder Finder;
            auto Matcher = expr(
                               allOf(
                                   unless(
                                       anyOf(
                                           implicitCastExpr(),
                                           implicitValueInitExpr())),
                                   anyOf(
                                       conditionalOperator(),
                                       binaryOperator(
                                           anyOf(
                                               hasOperatorName("&&"),
                                               hasOperatorName("||"))))))
                               .bind("root");
            StmtCollectorMatchHandler Handler;
            Finder.addMatcher(Matcher, &Handler);
            Finder.matchAST(Ctx);
            for (auto &&ST : Handler.Stmts)
                if (auto E = clang::dyn_cast<clang::Expr>(ST))
                    ConditionalExprs.insert(E);
        }

        // Any expr with a type defined at a local scope
        std::set<const clang::Expr *> ExprsWithLocallyDefinedTypes;
        {
            MatchFinder Finder;
            auto Matcher = expr(
                               unless(
                                   anyOf(
                                       implicitCastExpr(),
                                       implicitValueInitExpr())))
                               .bind("root");
            StmtCollectorMatchHandler Handler;
            Finder.addMatcher(Matcher, &Handler);
            Finder.matchAST(Ctx);
            for (auto &&ST : Handler.Stmts)
            {
                auto E = clang::dyn_cast<clang::Expr>(ST);
                auto QT = E->getType();
                if (hasLocalType(QT.getTypePtrOrNull(), Ctx))
                    ExprsWithLocallyDefinedTypes.insert(E);
            }
        }

        // Print macro expansion information
        for (auto Exp : MF->Expansions)
        {
            assert(Exp);
            assert(Exp->MI);

            // String properties
            std::string
                Name,
                DefinitionLocation,
                EndDefinitionLocation,
                InvocationLocation,
                ASTKind,
                TypeSignature;

            // Integer properties
            int
                InvocationDepth,
                NumASTRoots,
                NumArguments;

            // Boolean properties
            bool
                HasStringification,
                HasTokenPasting,
                HasAlignedArguments,
                HasSameNameAsOtherDeclaration,

                IsExpansionControlFlowStmt,

                DoesBodyReferenceMacroDefinedAfterMacro,
                DoesBodyReferenceDeclDeclaredAfterMacro,
                DoesBodyContainDeclRefExpr,
                DoesSubexpressionExpandedFromBodyHaveLocalType,
                DoesSubexpressionExpandedFromBodyHaveTypeDefinedAfterMacro,

                DoesAnyArgumentHaveSideEffects,
                DoesAnyArgumentContainDeclRefExpr,

                IsHygienic,
                IsDefinitionLocationValid,
                IsInvocationLocationValid,
                IsObjectLike,
                IsInvokedInMacroArgument,
                IsNamePresentInCPPConditional,
                IsExpansionICE,

                IsExpansionTypeNull,
                IsExpansionTypeAnonymous,
                IsExpansionTypeLocalType,
                IsExpansionTypeDefinedAfterMacro,
                IsExpansionTypeVoid,

                IsAnyArgumentTypeNull,
                IsAnyArgumentTypeAnonymous,
                IsAnyArgumentTypeLocalType,
                IsAnyArgumentTypeDefinedAfterMacro,
                IsAnyArgumentTypeVoid,

                IsInvokedWhereModifiableValueRequired,
                IsInvokedWhereAddressableValueRequired,
                IsInvokedWhereICERequired,

                IsAnyArgumentExpandedWhereModifiableValueRequired,
                IsAnyArgumentExpandedWhereAddressableValueRequired,
                IsAnyArgumentConditionallyEvaluated,
                IsAnyArgumentNeverExpanded,
                IsAnyArgumentNotAnExpression;

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
                    [&SM, &Exp](std::pair<std::string,
                                          const clang::MacroDirective *>
                                    Entry)
                    {
                        return SM.isBeforeInTranslationUnit(
                                   SM.getFileLoc(Entry.second
                                                     ->getDefinition()
                                                     .getLocation()),
                                   SM.getFileLoc(Exp->MI
                                                     ->getDefinitionLoc())) &&
                               std::any_of(
                                   Exp->Arguments.begin(),
                                   Exp->Arguments.end(),
                                   [&Entry](MacroExpansionArgument Arg)
                                   {
                                       return Arg.Name.str() == Entry.first;
                                   });
                    }) ||
                // Also check if any global declarations defined before this macro
                // have the same name as this macro
                std::any_of(
                    TopLevelDecls.begin(),
                    TopLevelDecls.end(),
                    [&SM, &Exp](const clang::Decl *D)
                    {
                        auto ND = clang::dyn_cast_or_null<clang::NamedDecl>(D);
                        if (!ND)
                            return false;
                        auto II = ND->getIdentifier();
                        if (!II)
                            return false;
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
            auto Res = tryGetFullSourceLoc(SM, Exp->MI->getDefinitionLoc());
            IsDefinitionLocationValid = Res.first;
            if (IsDefinitionLocationValid)
                DefinitionLocation = Res.second;
            auto [IsEndDefinitonLocationValid, EndLoc] = tryGetFullSourceLoc(SM, Exp->MI->getDefinitionEndLoc());
            if(IsEndDefinitonLocationValid)
                EndDefinitionLocation = EndLoc;
                

            // Invocation location
            Res = tryGetFullSourceLoc(SM, Exp->SpellingRange.getBegin());
            IsInvocationLocationValid = Res.first;
            if (IsInvocationLocationValid)
                InvocationLocation = Res.second;

            auto DefLoc = SM.getFileLoc(Exp->MI->getDefinitionLoc());

            // Check if any macro this macro invokes were defined after
            // this macro was
            auto Descendants = Exp->getDescendants();

            DoesBodyReferenceMacroDefinedAfterMacro = std::any_of(
                Descendants.begin(),
                Descendants.end(),
                [&SM, &Exp](MacroExpansionNode *Desc)
                { return SM.isBeforeInTranslationUnit(
                      SM.getFileLoc(Exp->MI->getDefinitionLoc()),
                      SM.getFileLoc(Desc->MI->getDefinitionLoc())); });

            // Next get AST information for top level invocations
            if (Exp->Depth == 0 && !Exp->InMacroArg)
            {
                debug("Top level invocation: ", Exp->Name.str());
                cpp2c::findAlignedASTNodesForExpansion(Exp, Ctx);

                //// Print macro info

                // Exp->dumpMacroInfo(llvm::outs());

                // Exp->dumpASTInfo(llvm::outs(),
                //                  Ctx.getSourceManager(), Ctx.getLangOpts());

                // Number of AST roots
                NumASTRoots = Exp->ASTRoots.size();

                // Determine the AST kind of the expansion
                debug("Checking if expansion has aligned root");
                if (Exp->AlignedRoot)
                {
                    auto D = Exp->AlignedRoot->D;
                    auto ST = Exp->AlignedRoot->ST;
                    auto TL = Exp->AlignedRoot->TL;

                    if (ST)
                    {
                        debug("Aligns with a stmt");
                        ASTKind = "Stmt";
                    }
                    else if (D)
                    {
                        debug("Aligns with a decl");
                        ASTKind = "Decl";
                    }
                    else if (TL)
                    {
                        debug("Aligns with a type loc");
                        ASTKind = "TypeLoc";
                        // Check that this type specifier list does not include
                        // a typedef that was defined after the macro was defined
                        // debug("Checking if type loc type is null");
                        IsExpansionTypeNull = TL->isNull();

                        // FIXME: For some reason, this function call sometimes
                        // triggers an error. I have tried to debug it the best
                        // I can, but it seems to be due to a problem with
                        // Clang.
                        // Until this is fixed, we will not be able to gather
                        // full data on TypeLocs.
                        // debug("Checking hasTypeDefinedAfter");
                        // IsExpansionTypeDefinedAfterMacro = (!TL->isNull()) &&
                        //     hasTypeDefinedAfter(TL->getTypePtr(), Ctx, DefLoc);
                        // debug("Finished checking hasTypeDefinedAfter");
                    }
                    else
                        assert("Aligns with node that is not a Decl/Stmt/TypeLoc");
                }

                // Check that the number of AST nodes aligned with each argument
                // equals the number of times that argument was expanded
                debug("Checking if arguments are all aligned");
                HasAlignedArguments = std::all_of(
                    Exp->Arguments.begin(),
                    Exp->Arguments.end(),
                    [](MacroExpansionArgument Arg)
                    { return Arg.AlignedRoots.size() == Arg.NumExpansions; });
                debug("Done checking if arguments are all aligned");

                std::set<const clang::Stmt *> StmtsExpandedFromArguments;
                // Semantic properties of the macro's arguments
                if (HasAlignedArguments)
                {
                    debug("Collecting argument subtrees");
                    for (auto &&Arg : Exp->Arguments)
                    {
                        for (auto &&Root : Arg.AlignedRoots)
                        {
                            auto STs = subtrees(Root.ST);
                            StmtsExpandedFromArguments.insert(STs.begin(), STs.end());
                        }
                    }
                    debug("Done collecting argument subtrees");

                    auto ExpandedFromArgument =
                        [&StmtsExpandedFromArguments](const clang::Stmt *St)
                    { return StmtsExpandedFromArguments.find(St) !=
                             StmtsExpandedFromArguments.end(); };

                    DoesAnyArgumentHaveSideEffects = std::any_of(
                        SideEffectExprs.begin(),
                        SideEffectExprs.end(),
                        ExpandedFromArgument);

                    DoesAnyArgumentContainDeclRefExpr = std::any_of(
                        AllDeclRefExprs.begin(),
                        AllDeclRefExprs.end(),
                        ExpandedFromArgument);

                    IsAnyArgumentExpandedWhereModifiableValueRequired = std::any_of(
                        SideEffectExprs.begin(),
                        SideEffectExprs.end(),
                        [&ExpandedFromArgument](const clang::Expr *E)
                        {
                            // Only consider side-effect expressions which were
                            // not expanded from an argument of the same macro
                            if (!ExpandedFromArgument(E))
                            {
                                clang::Expr *LHS = nullptr;
                                auto B = clang::dyn_cast<clang::BinaryOperator>(E);
                                auto U = clang::dyn_cast<clang::UnaryOperator>(E);
                                if (B)
                                    LHS = B->getLHS();
                                else if (U)
                                    LHS = U->getSubExpr();
                                LHS = skipImplicitAndParens(LHS);
                                return ExpandedFromArgument(LHS);
                            }
                            return false;
                        });

                    IsAnyArgumentExpandedWhereAddressableValueRequired = std::any_of(
                        AddressOfExprs.begin(),
                        AddressOfExprs.end(),
                        [&ExpandedFromArgument](const clang::UnaryOperator *U)
                        {
                            // Only consider address of expressions which were
                            // not expanded from an argument of the same macro
                            if (!ExpandedFromArgument(U))
                            {
                                auto Operand = U->getSubExpr();
                                Operand = skipImplicitAndParens(Operand);
                                return ExpandedFromArgument(Operand);
                            }
                            return false;
                        });
                }

                std::set<const clang::Stmt *> StmtsExpandedFromBody;
                // Semantic properties of the macro body
                if (Exp->AlignedRoot && Exp->AlignedRoot->ST && HasAlignedArguments)
                {
                    auto ST = Exp->AlignedRoot->ST;

                    debug("Collecting body subtrees");
                    StmtsExpandedFromBody = subtrees(ST);
                    // Remove all Stmts which were actually expanded from arguments
                    for (auto &&St : StmtsExpandedFromArguments)
                        StmtsExpandedFromBody.erase(St);

                    auto ExpandedFromBody =
                        [&StmtsExpandedFromBody](const clang::Stmt *St)
                    { return StmtsExpandedFromBody.find(St) !=
                             StmtsExpandedFromBody.end(); };

                    debug("Checking if any argument is conditionally "
                            "evaluated in the body of the expansion");
                    IsAnyArgumentConditionallyEvaluated = std::any_of(
                        ConditionalExprs.begin(),
                        ConditionalExprs.end(),
                        [&ExpandedFromBody,
                         &StmtsExpandedFromArguments](const clang::Expr *CE)
                        {
                            return ExpandedFromBody(CE) && std::any_of(
                                StmtsExpandedFromArguments.begin(),
                                StmtsExpandedFromArguments.end(),
                                [&CE](const clang::Stmt *ArgStmt)
                                { return inTree(ArgStmt, CE); });
                        });
                    debug("Done checking if any argument is conditionally "
                            "evaluated in the body of the expansion");

                    // NOTE: This may not be correct if the definition of
                    // of the decl is separate from its declaration.
                    DoesBodyReferenceDeclDeclaredAfterMacro = std::any_of(
                        AllDeclRefExprs.begin(),
                        AllDeclRefExprs.end(),
                        [&SM,
                         &DefLoc,
                         &ExpandedFromBody](const clang::DeclRefExpr *DRE)
                        {
                            if (ExpandedFromBody(DRE))
                            {
                                auto D = DRE->getDecl();
                                auto DeclLoc = SM.getFileLoc(D->getLocation());

                                return SM.isBeforeInTranslationUnit(DefLoc,
                                                                    DeclLoc);
                            }
                            return false;
                        });

                    DoesBodyContainDeclRefExpr = std::any_of(
                        AllDeclRefExprs.begin(),
                        AllDeclRefExprs.end(),
                        ExpandedFromBody);

                    DoesSubexpressionExpandedFromBodyHaveLocalType = std::any_of(
                        ExprsWithLocallyDefinedTypes.begin(),
                        ExprsWithLocallyDefinedTypes.end(),
                        ExpandedFromBody);

                    DoesSubexpressionExpandedFromBodyHaveTypeDefinedAfterMacro =
                        std::any_of(
                            StmtsExpandedFromBody.begin(),
                            StmtsExpandedFromBody.end(),
                            [&Ctx, &DefLoc](const clang::Stmt *St)
                            {
                                if (auto E = clang::dyn_cast<clang::Expr>(St))
                                {
                                    auto QT = E->getType();
                                    return hasTypeDefinedAfter(QT.getTypePtrOrNull(), Ctx, DefLoc);
                                }
                                return false;
                            });

                    // We only allow references to declarations declared
                    // within the macro expansion itself
                    IsHygienic = std::none_of(
                        DeclRefExprsOfLocallyDefinedDecls.begin(),
                        DeclRefExprsOfLocallyDefinedDecls.end(),
                        [&ST, &SM, &ExpandedFromBody](const clang::DeclRefExpr *DRE)
                        {
                            // References that don't come from the macro's body
                            // are fine
                            if (!ExpandedFromBody(DRE))
                                return false;

                            auto B = SM.getFileLoc(ST->getBeginLoc());
                            auto E = SM.getFileLoc(ST->getEndLoc());
                            auto D = DRE->getDecl();
                            if (!D)
                                return false;

                            auto L = SM.getFileLoc(D->getLocation());
                            // NOTE: It would be nice if we could instead walk
                            // the AST and check if this decl is under the AST
                            // aligned with this macro.
                            // This should work for now though.
                            return !clang::SourceRange(B, E).fullyContains(L);
                        });

                    IsInvokedWhereModifiableValueRequired = std::any_of(
                        SideEffectExprs.begin(),
                        SideEffectExprs.end(),
                        [&ST, &ExpandedFromBody](const clang::Expr *E)
                        {
                            // Only consider side-effect expressions which were
                            // not expanded from the body of the same macro
                            if (!ExpandedFromBody(E))
                            {
                                clang::Expr *LHS = nullptr;
                                auto B = clang::dyn_cast<clang::BinaryOperator>(E);
                                auto U = clang::dyn_cast<clang::UnaryOperator>(E);
                                if (B)
                                    LHS = B->getLHS();
                                else if (U)
                                    LHS = U->getSubExpr();
                                return inTree(ST, LHS);
                            }
                            return false;
                        });

                    IsInvokedWhereAddressableValueRequired = std::any_of(
                        AddressOfExprs.begin(),
                        AddressOfExprs.end(),
                        [&ST, &ExpandedFromBody](const clang::UnaryOperator *U)
                        {
                            // Only consider address of expressions which were
                            // not expanded from the body of the same macro
                            if (!ExpandedFromBody(U))
                            {
                                auto Operand = U->getSubExpr();
                                Operand = skipImplicitAndParens(Operand);
                                return inTree(ST, Operand);
                            }
                            return false;
                        });

                    IsInvokedWhereICERequired =
                        isDescendantOfStmtRequiringICE(Ctx, ST);

                    //// Generate type signature

                    // Body type information
                    TypeSignature = "void";
                    if (auto E = clang::dyn_cast<clang::Expr>(ST))
                    {
                        ASTKind = "Expr";

                        // Type information about the entire expansion
                        auto QT = E->getType();
                        auto T = QT.getTypePtrOrNull();
                        IsExpansionTypeNull = QT.isNull() || T == nullptr;

                        if (T)
                        {
                            IsExpansionTypeVoid = T->isVoidType();
                            IsExpansionTypeAnonymous = hasAnonymousType(T, Ctx);
                            IsExpansionTypeLocalType = hasLocalType(T, Ctx);
                            auto CT = QT.getDesugaredType(Ctx)
                                          .getUnqualifiedType()
                                          .getCanonicalType();
                            TypeSignature = CT.getAsString();
                        }
                        IsExpansionTypeDefinedAfterMacro =
                            hasTypeDefinedAfter(QT.getTypePtrOrNull(), Ctx, DefLoc);

                        // Whether this expression is an integral
                        // constant expression
                        IsExpansionICE = E->isIntegerConstantExpr(Ctx);
                    }

                    // Argument type information
                    IsAnyArgumentNotAnExpression = false;
                    IsAnyArgumentTypeNull = false;
                    IsAnyArgumentTypeDefinedAfterMacro = false;

                    if (Exp->MI->isFunctionLike() &&
                        (ASTKind == "Stmt" || ASTKind == "Expr"))
                        TypeSignature += "(";
                    debug("Iterating arguments");
                    int ArgNum = 0;
                    for (auto &&Arg : Exp->Arguments)
                    {
                        if (ArgNum != 0)
                            TypeSignature += ", ";
                        ArgNum += 1;

                        IsAnyArgumentNeverExpanded = Arg.AlignedRoots.empty();

                        if (Arg.AlignedRoots.empty())
                            continue;

                        auto Arg1stExpST = Arg.AlignedRoots.front().ST;
                        auto E = clang::dyn_cast_or_null<clang::Expr>(Arg1stExpST);

                        IsAnyArgumentNotAnExpression |= (E == nullptr);

                        debug("Checking if argument is an expression");

                        if (!E)
                            continue;

                        std::string ArgTypeStr = "<Null>";

                        // Type information about arguments
                        auto QT = E->getType();
                        auto T = QT.getTypePtrOrNull();
                        IsAnyArgumentTypeNull |= QT.isNull() || T == nullptr;

                        if (T)
                        {
                            IsAnyArgumentTypeVoid = T->isVoidType();
                            IsAnyArgumentTypeAnonymous = hasAnonymousType(T, Ctx);
                            IsAnyArgumentTypeLocalType = hasLocalType(T, Ctx);
                            auto CT = QT.getDesugaredType(Ctx)
                                          .getUnqualifiedType()
                                          .getCanonicalType();
                            ArgTypeStr = CT.getAsString();
                        }
                        IsAnyArgumentTypeDefinedAfterMacro |=
                            hasTypeDefinedAfter(QT.getTypePtrOrNull(), Ctx, DefLoc);

                        TypeSignature += ArgTypeStr;
                    }
                    debug("Finished iterating arguments");
                    if (Exp->MI->isFunctionLike() &&
                        (ASTKind == "Stmt" || ASTKind == "Expr"))
                        TypeSignature += ")";
                }

                // Set of all Stmts expanded from macro
                std::set<const clang::Stmt *> AllStmtsExpandedFromMacro =
                    StmtsExpandedFromBody;
                AllStmtsExpandedFromMacro.insert(StmtsExpandedFromArguments.begin(),
                                                 StmtsExpandedFromArguments.end());

                IsExpansionControlFlowStmt = std::any_of(
                    AllStmtsExpandedFromMacro.begin(),
                    AllStmtsExpandedFromMacro.end(),
                    [](const clang::Stmt *St)
                    {
                        return llvm::isa_and_nonnull<clang::ReturnStmt>(St) ||
                               llvm::isa_and_nonnull<clang::ContinueStmt>(St) ||
                               llvm::isa_and_nonnull<clang::BreakStmt>(St) ||
                               llvm::isa_and_nonnull<clang::GotoStmt>(St);
                    });
            }

            auto entryString = [](std::string k, std::string v) -> std::string
            {
                return "    \"" + k + "\" : \"" + v + "\"";
            };

            auto entryInt = [](std::string k, int v) -> std::string
            {
                return "    \"" + k + "\" : " + std::to_string(v);
            };

            auto entryBool = [](std::string k, bool v) -> std::string
            {
                return "    \"" + k + "\" : " + (v ? "true" : "false");
            };

            std::vector<std::pair<std::string, std::string>> stringEntries =
                {
                    {"Name", Name},
                    {"DefinitionLocation", DefinitionLocation},
                    {"EndDefinitionLocation", EndDefinitionLocation},
                    {"InvocationLocation", InvocationLocation},
                    {"ASTKind", ASTKind},
                    {"TypeSignature", TypeSignature},
                };

            std::vector<std::pair<std::string, int>> intEntries =
                {
                    {"InvocationDepth", InvocationDepth},
                    {"NumASTRoots", NumASTRoots},
                    {"NumArguments", NumArguments},
                };

            std::vector<std::pair<std::string, bool>> boolEntries =
                {
                    {"HasStringification", HasStringification},
                    {"HasTokenPasting", HasTokenPasting},
                    {"HasAlignedArguments", HasAlignedArguments},
                    {"HasSameNameAsOtherDeclaration", HasSameNameAsOtherDeclaration},

                    {"IsExpansionControlFlowStmt", IsExpansionControlFlowStmt},

                    {"DoesBodyReferenceMacroDefinedAfterMacro", DoesBodyReferenceMacroDefinedAfterMacro},
                    {"DoesBodyReferenceDeclDeclaredAfterMacro", DoesBodyReferenceDeclDeclaredAfterMacro},
                    {"DoesBodyContainDeclRefExpr", DoesBodyContainDeclRefExpr},
                    {"DoesSubexpressionExpandedFromBodyHaveLocalType", DoesSubexpressionExpandedFromBodyHaveLocalType},
                    {"DoesSubexpressionExpandedFromBodyHaveTypeDefinedAfterMacro", DoesSubexpressionExpandedFromBodyHaveTypeDefinedAfterMacro},

                    {"DoesAnyArgumentHaveSideEffects", DoesAnyArgumentHaveSideEffects},
                    {"DoesAnyArgumentContainDeclRefExpr", DoesAnyArgumentContainDeclRefExpr},

                    {"IsHygienic", IsHygienic},
                    {"IsDefinitionLocationValid", IsDefinitionLocationValid},
                    {"IsInvocationLocationValid", IsInvocationLocationValid},
                    {"IsObjectLike", IsObjectLike},
                    {"IsInvokedInMacroArgument", IsInvokedInMacroArgument},
                    {"IsNamePresentInCPPConditional", IsNamePresentInCPPConditional},
                    {"IsExpansionICE", IsExpansionICE},

                    {"IsExpansionTypeNull", IsExpansionTypeNull},
                    {"IsExpansionTypeAnonymous", IsExpansionTypeAnonymous},
                    {"IsExpansionTypeLocalType", IsExpansionTypeLocalType},
                    {"IsExpansionTypeDefinedAfterMacro", IsExpansionTypeDefinedAfterMacro},
                    {"IsExpansionTypeVoid", IsExpansionTypeVoid},

                    {"IsAnyArgumentTypeNull", IsAnyArgumentTypeNull},
                    {"IsAnyArgumentTypeAnonymous", IsAnyArgumentTypeAnonymous},
                    {"IsAnyArgumentTypeLocalType", IsAnyArgumentTypeLocalType},
                    {"IsAnyArgumentTypeDefinedAfterMacro", IsAnyArgumentTypeDefinedAfterMacro},
                    {"IsAnyArgumentTypeVoid", IsAnyArgumentTypeVoid},

                    {"IsInvokedWhereModifiableValueRequired", IsInvokedWhereModifiableValueRequired},
                    {"IsInvokedWhereAddressableValueRequired", IsInvokedWhereAddressableValueRequired},
                    {"IsInvokedWhereICERequired", IsInvokedWhereICERequired},

                    {"IsAnyArgumentExpandedWhereModifiableValueRequired", IsAnyArgumentExpandedWhereModifiableValueRequired},
                    {"IsAnyArgumentExpandedWhereAddressableValueRequired", IsAnyArgumentExpandedWhereAddressableValueRequired},
                    {"IsAnyArgumentConditionallyEvaluated", IsAnyArgumentConditionallyEvaluated},
                    {"IsAnyArgumentNeverExpanded", IsAnyArgumentNeverExpanded},
                    {"IsAnyArgumentNotAnExpression", IsAnyArgumentNotAnExpression},
                };

            // Only pretty print JSON if debug is on
            const char sep = Debug ? '\n' : ' ';

            llvm::outs() << "Invocation" << delim << '{' << sep;
            for (auto &&e : stringEntries)
                llvm::outs() << entryString(e.first, e.second) << "," << sep;
            for (auto &&e : intEntries)
                llvm::outs() << entryInt(e.first, e.second) << "," << sep;
            for (size_t i = 0; i < boolEntries.size(); i++)
            {
                auto e = boolEntries[i];
                llvm::outs() << entryBool(e.first, e.second)
                             << (i == (boolEntries.size() - 1) ? "" : ",")
                             << sep;
            }
            llvm::outs() << " }\n";
        }

        // Only delete top level expansions since deconstructor deletes
        // nested expansions
        for (auto &&Exp : MF->Expansions)
            if (Exp->Depth == 0)
                delete Exp;
    }
} // namespace cpp2c
