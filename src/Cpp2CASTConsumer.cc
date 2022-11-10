#include "Cpp2CASTConsumer.hh"
#include "ASTUtils.hh"
#include "DeclStmtTypeLoc.hh"
#include "DeclCollectorMatchHandler.hh"
#include "ExpansionMatchHandler.hh"
#include "AlignmentMatchers.hh"
#include "Properties.hh"
#include "IncludeCollector.hh"

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

// TODO: Commit changes so far!
// TODO: Review checks for parameter side-effects and l-value independence.
//       Some of the results so far for Lua don't look right.
//       For example, if the macro applies the arrow operator to its argument,
//       this should not cause the macro to be labeled l-value dependent.

namespace cpp2c
{
    static const constexpr bool Debug = false;
    static const constexpr char *delim = "\t";
    inline std::string fmt(std::string s) { return s; }
    inline std::string fmt(const char *s) { return std::string(s); }
    inline std::string fmt(bool b) { return b ? "T" : "F"; }
    inline std::string fmt(unsigned int i) { return std::to_string(i); }

    template <typename T>
    inline void print(T t) { llvm::outs() << fmt(t) << "\n"; }

    template <typename T1, typename T2, typename... Ts>
    inline void print(T1 t1, T2 t2, Ts... ts)
    {
        llvm::outs() << fmt(t1) << delim;
        print(t2, ts...);
    }

    inline void delimit() { print(delim); }

    template <typename... Ts>
    inline void debug(Ts... ts)
    {
        if (Debug)
            print(ts...);
    }

    struct MacroFacts
    {
        std::string
            Name = "N/A",
            DefLocOrError = "N/A",
            InvokeLocOrError = "N/A",
            ASTKind = "N/A",
            TypeSignature = "N/A";

        unsigned int
            Depth = 404,
            NumASTRoots = 40,
            NumArguments = 404;

        bool
            // All macros
            IsObjectLike = false,
            InMacroArg = false,
            ValidDefLoc = false,
            ValidInvokeLoc = false,
            HasStringification = false,
            HasTokenPasting = false,
            InvokesLaterDefinedMacro = false,
            IsNameInspectedByPreprocessor = false,

            // Stmt/Expr macros
            IsIntConstExpr = false,
            ContainsDeclRefExpr = false,
            ContainsConditionalEvaluation = false,
            ContainsSubExprFromBodyWithLocalOrAnonymousType = false,
            ContainsDeclFromBodyDefinedAfterMacro = false,
            ContainsSubExprFromBodyWithTypeDefinedAfterMacro = false,
            ExpansionHasType = false,

            // Semantic properties for Stmt/Expr macros
            IsHygienic = false,
            IsParameterSideEffectFree = false,
            IsLValueIndependent = false,

            // Expr macros whose type is not null
            IsExpansionVoidType = false,
            IsExpansionLocalOrAnonymousType = false,
            ExpansionContainsTypeDefinedAfterMacroWasDefined = false,
            ExpandedWhereConstExprRequired = false,

            // TypeLoc macros
            IsNullType = false,

            // TypeLoc macros whose type is not null
            TypeLocContainsTypeDefinedAfterMacroWasDefined = false,

            // Macro arguments
            HasAlignedArguments = false,
            HasUnexpandedArgument = false,
            HasNonExprArgument = false,
            HasUntypedArgument = false,
            HasArgumentWithVoidType = false,
            HasArgumentWithLocalOrAnonymousType = false,
            HasArgumentWithTypeDefinedAfterMacro = false;
    };

    void printFacts(struct MacroFacts F)
    {
        print("Invocation",
              F.Name,
              F.DefLocOrError,
              F.InvokeLocOrError,
              F.ASTKind,
              F.TypeSignature,

              F.Depth,
              F.NumASTRoots,
              F.NumArguments,

              F.IsObjectLike,
              F.InMacroArg,
              F.ValidDefLoc,
              F.ValidInvokeLoc,
              F.HasStringification,
              F.HasTokenPasting,
              F.InvokesLaterDefinedMacro,
              F.IsNameInspectedByPreprocessor,

              F.IsIntConstExpr,
              F.ContainsDeclRefExpr,
              F.ContainsConditionalEvaluation,
              F.ContainsSubExprFromBodyWithLocalOrAnonymousType,
              F.ContainsDeclFromBodyDefinedAfterMacro,
              F.ContainsSubExprFromBodyWithTypeDefinedAfterMacro,
              F.ExpansionHasType,

              F.IsHygienic,
              F.IsParameterSideEffectFree,
              F.IsLValueIndependent,

              F.IsExpansionVoidType,
              F.IsExpansionLocalOrAnonymousType,
              F.ExpansionContainsTypeDefinedAfterMacroWasDefined,
              F.ExpandedWhereConstExprRequired,

              F.IsNullType,

              F.TypeLocContainsTypeDefinedAfterMacroWasDefined,

              F.HasAlignedArguments,
              F.HasUnexpandedArgument,
              F.HasNonExprArgument,
              F.HasUntypedArgument,
              F.HasArgumentWithVoidType,
              F.HasArgumentWithLocalOrAnonymousType,
              F.HasArgumentWithTypeDefinedAfterMacro);
    }

    template <typename T>
    inline void printIfIsOneOf(const clang::Stmt *ST)
    {
        if (clang::isa<T>(ST))
            print(typeid(T).name());
    }

    template <typename T1, typename T2, typename... Ts>
    inline void printIfIsOneOf(const clang::Stmt *ST)
    {
        if (clang::isa<T1>(ST))
            print(typeid(T1).name());
        printIfIsOneOf<T2, Ts...>(ST);
    }

    inline std::function<bool(const clang::Stmt *)>
    stmtIsBinOp(clang::BinaryOperator::Opcode OC)
    {
        return [OC](const clang::Stmt *ST)
        {
            if (auto BO = clang::dyn_cast<clang::BinaryOperator>(ST))
                return BO->getOpcode() == OC;
            return false;
        };
    }

    bool descendantOfConstantExpression(clang::ASTContext &Ctx,
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

    // Returns a set of all Stmts parsed from the given expansion's arguments
    std::set<const clang::Stmt *>
    collectStmtsFromArguments(MacroExpansionNode *Expansion)
    {
        assert(Expansion);

        std::set<const clang::Stmt *> Result;
        for (auto &&Arg : Expansion->Arguments)
        {
            for (auto &&R : Arg.AlignedRoots)
            {
                if (R.ST)
                {
                    std::queue<const clang::Stmt *> Q;
                    Q.push(R.ST);
                    while (!(Q.empty()))
                    {
                        auto Cur = Q.front();
                        Q.pop();

                        if (!Cur)
                            continue;

                        Result.insert(Cur);
                        for (auto &&child : Cur->children())
                            Q.push(child);
                    }
                }
            }
        }
        return Result;
    }

    // Returns true if the given predicate returns true for any type
    // contained in the given type
    bool isInType(
        const clang::QualType QT,
        std::function<bool(const clang::Type *)> pred)
    {
        debug("Calling isInType");

        if (QT.isNull())
            return false;

        auto T = QT.getTypePtrOrNull();

        while (T != nullptr &&
               // NOTE: I'm not sure why we need this check, but if I remove
               // it then Clang may crash on certain inputs.
               // See tests/declare_bitmap.c
               (!T->getCanonicalTypeInternal().isNull()) &&
               (T->isAnyPointerType() || T->isArrayType()))
            T = T->getPointeeOrArrayElementType();

        debug("(isInType) calling pred");
        return pred(T);
    }

    // Returns true if any type in T was defined after L
    inline bool containsTypeDefinedAfter(
        const clang::QualType QT,
        clang::SourceManager &SM,
        clang::SourceLocation L)
    {
        return isInType(
            QT,
            [&SM, L](const clang::Type *Ty)
            {
                debug("(isInType) Checking Ty");
                if (!Ty)
                    return false;

                debug("(isInType) Checking internal qualified type");
                if (Ty->getCanonicalTypeInternal().isNull())
                    return false;

                debug("(isInType) Getting TD");
                auto TD = Ty->getAsTagDecl();
                debug("(isInType) Checking TD");
                if (!TD)
                    return false;

                debug("(isInType) Checking TDLoc");
                auto TDLoc = TD->getLocation();
                if (TDLoc.isInvalid())
                    return false;

                debug("(isInType) Checking DeclFLoc");
                auto DeclFLoc = SM.getFileLoc(TDLoc);
                if (DeclFLoc.isInvalid())
                    return false;

                return SM.isBeforeInTranslationUnit(L, DeclFLoc);
            });
    }

    // Adds the definition location of each invoked macro in the given
    // expansion tree to the given vector of SourceLocations
    void collectExpansionDefLocs(
        clang::SourceManager &SM,
        std::vector<clang::SourceLocation> &DefLocs,
        cpp2c::MacroExpansionNode *Expansion)
    {
        if (nullptr == Expansion)
            return;

        assert(Expansion->MI);

        auto L = SM.getFileLoc(Expansion->MI->getDefinitionLoc());
        DefLocs.push_back(L);

        for (auto &&Child : Expansion->Children)
            collectExpansionDefLocs(SM, DefLocs, Child);
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
        std::pair<const clang::FileEntry *, clang::SourceLocation> &IEL,
        std::set<llvm::StringRef> &LocalIncludes,
        std::vector<const clang::Decl *> &Decls)
    {
        auto FE = IEL.first;
        auto HashLoc = IEL.second;

        // Check that the included file is not null
        if (!FE)
            return {false, "<null>"};

        // Check that the included file actually has a name
        auto IncludedFileRealpath = FE->tryGetRealPathName();
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
                        if (Tok.hasValue())
                            E = SM.getFileLoc(Tok.getValue().getEndLoc());

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

            print("Definition", Name, Valid, DefLocOrError);
        }

        // Print names of macros inspected by the preprocessor
        for (auto &&Name : DC->InspectedMacroNames)
            print("InspectedByCPP", Name);

        // Print include-directive information
        {
            // Collect declaration ranges
            std::vector<const clang::Decl *> Decls =
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

            std::set<llvm::StringRef> LocalIncludes;
            for (auto &&IEL : IC->IncludeEntriesLocs)
            {
                // Facts for includes
                bool Valid = false;
                std::string IncludeName = "";

                // Check if included at global scope or not
                auto Res = isGlobalInclude(SM, LO, IEL, LocalIncludes, Decls);
                if (!Res.first)
                    LocalIncludes.insert(Res.second);

                Valid = Res.first;
                IncludeName = Res.second.empty() ? "" : Res.second.str();

                print("Include", Valid, IncludeName);
            }
        }

        // Print macro expansion information
        for (auto Exp : MF->Expansions)
        {
            assert(Exp);
            assert(Exp->MI);

            debug("Checking", Exp->Name.str());

            //// First get basic info about all macros
            struct MacroFacts Facts = (struct MacroFacts){
                .Name = Exp->Name.str(),

                .Depth = Exp->Depth,

                .IsObjectLike = Exp->MI->isObjectLike(),
                .InMacroArg = Exp->InMacroArg,
                .HasStringification = Exp->HasStringification,
                .HasTokenPasting = Exp->HasTokenPasting,
                .IsNameInspectedByPreprocessor =
                    DC->InspectedMacroNames.find(Exp->Name.str()) !=
                    DC->InspectedMacroNames.end()

            };

            // Definition location
            auto Res = tryGetFullSourceLoc(SM, Exp->MI->getDefinitionLoc());
            Facts.ValidDefLoc = Res.first;
            Facts.DefLocOrError = Res.second;

            // Invocation location
            Res = tryGetFullSourceLoc(SM, Exp->SpellingRange.getBegin());
            Facts.ValidInvokeLoc = Res.first;
            Facts.InvokeLocOrError = Res.second;

            auto DefLoc = SM.getFileLoc(Exp->MI->getDefinitionLoc());
            // Check if any macros this macro invokes were defined after
            // this macro was
            Facts.InvokesLaterDefinedMacro = ({
                std::vector<clang::SourceLocation> DescendantMacroDefLocs;
                for (auto &&Child : Exp->Children)
                    collectExpansionDefLocs(SM, DescendantMacroDefLocs, Child);
                std::any_of(
                    DescendantMacroDefLocs.begin(),
                    DescendantMacroDefLocs.end(),
                    [&SM, DefLoc](clang::SourceLocation L)
                    {
                        return SM.isBeforeInTranslationUnit(DefLoc, L);
                    });
            });

            // Stop here for nested macro invocations and macro arguments
            if (Exp->Depth != 0 || Exp->InMacroArg)
            {
                printFacts(Facts);
                continue;
            }

            // Next get AST information for top level invocations

            using namespace clang::ast_matchers;

            debug("Matching stmts");
            // Match stmts
            if (!Exp->DefinitionTokens.empty())
            {
                MatchFinder Finder;
                ExpansionMatchHandler Handler;
                auto Matcher = stmt(unless(anyOf(implicitCastExpr(),
                                                 implicitValueInitExpr())),
                                    alignsWithExpansion(&Ctx, Exp))
                                   .bind("root");
                Finder.addMatcher(Matcher, &Handler);
                Finder.matchAST(Ctx);
                for (auto &&M : Handler.Matches)
                    Exp->ASTRoots.push_back(M);
            }
            debug("Finished matching stmts");

            debug("Matching decls");
            // Match decls
            if (!Exp->DefinitionTokens.empty())
            {
                MatchFinder Finder;
                ExpansionMatchHandler Handler;
                auto Matcher = decl(alignsWithExpansion(&Ctx, Exp))
                                   .bind("root");
                Finder.addMatcher(Matcher, &Handler);
                Finder.matchAST(Ctx);
                for (auto &&M : Handler.Matches)
                    Exp->ASTRoots.push_back(M);
            }
            debug("Finished matching decls");

            debug("Matching type locs");
            // Match type locs
            if (!Exp->DefinitionTokens.empty())
            {
                MatchFinder Finder;
                ExpansionMatchHandler Handler;
                auto Matcher = typeLoc(alignsWithExpansion(&Ctx, (Exp)))
                                   .bind("root");
                Finder.addMatcher(Matcher, &Handler);
                Finder.matchAST(Ctx);
                for (auto &&M : Handler.Matches)
                    Exp->ASTRoots.push_back(M);
            }
            debug("Finished matching type locs");

            // If the expansion only aligns with one node, then set this
            // as its aligned root
            Exp->AlignedRoot = (Exp->ASTRoots.size() == 1)
                                   ? (&(Exp->ASTRoots.front()))
                                   : nullptr;

            //// Find AST roots aligned with each of the expansion's arguments

            for (auto &&Arg : Exp->Arguments)
            {
                debug("Matching arg stmts");
                // Match stmts
                if (!(Arg.Tokens.empty()))
                {
                    MatchFinder Finder;
                    ExpansionMatchHandler Handler;
                    auto Matcher = stmt(unless(anyOf(implicitCastExpr(),
                                                     implicitValueInitExpr())),
                                        isSpelledFromTokens(&Ctx, Arg.Tokens))
                                       .bind("root");
                    Finder.addMatcher(Matcher, &Handler);
                    Finder.matchAST(Ctx);
                    for (auto &&M : Handler.Matches)
                        Arg.AlignedRoots.push_back(M);
                }
                debug("Finished matching arg stmts");

                debug("Matching arg decls");
                // Match decls
                if (!(Arg.Tokens.empty()))
                {
                    MatchFinder Finder;
                    ExpansionMatchHandler Handler;
                    auto Matcher = decl(isSpelledFromTokens(&Ctx, Arg.Tokens))
                                       .bind("root");
                    Finder.addMatcher(Matcher, &Handler);
                    Finder.matchAST(Ctx);
                    for (auto &&M : Handler.Matches)
                        Arg.AlignedRoots.push_back(M);
                }
                debug("Finished matching arg decls");

                debug("Matching arg type locs");
                // Match type locs
                if (!(Arg.Tokens.empty()))
                {
                    MatchFinder Finder;
                    ExpansionMatchHandler Handler;
                    auto Matcher =
                        typeLoc(isSpelledFromTokens(&Ctx, Arg.Tokens))
                            .bind("root");
                    Finder.addMatcher(Matcher, &Handler);
                    Finder.matchAST(Ctx);
                    for (auto &&M : Handler.Matches)
                        Arg.AlignedRoots.push_back(M);
                }
                debug("Finished matching arg type locs");
            }

            //// Print macro info

            // Exp->dumpMacroInfo(llvm::outs());

            // Exp->dumpASTInfo(llvm::outs(),
            //                  Ctx.getSourceManager(), Ctx.getLangOpts());

            // Number of AST roots
            Facts.NumASTRoots = Exp->ASTRoots.size();

            debug("Checking if expansion has aligned root");
            if (Exp->AlignedRoot)
            {
                auto D = Exp->AlignedRoot->D;
                auto ST = Exp->AlignedRoot->ST;
                auto TL = Exp->AlignedRoot->TL;

                if (ST)
                {
                    debug("Aligns with a stmt");
                    Facts.ASTKind = "Stmt";

                    // TODO: Add this to facts?
                    // printIfIsOneOf<clang::DoStmt,
                    //                clang::ContinueStmt,
                    //                clang::BreakStmt,
                    //                clang::ReturnStmt,
                    //                clang::GotoStmt,

                    //                clang::Expr,
                    //                clang::CharacterLiteral,
                    //                clang::IntegerLiteral,
                    //                clang::FloatingLiteral,
                    //                clang::FixedPointLiteral,
                    //                clang::ImaginaryLiteral,
                    //                clang::StringLiteral,
                    //                clang::CompoundLiteralExpr>(ST);

                    Facts.ContainsDeclRefExpr =
                        isInTree(ST, stmtIsA<clang::DeclRefExpr>());

                    auto LogicalAnd = clang::BinaryOperator::Opcode::BO_LAnd;
                    auto LogicalOr = clang::BinaryOperator::Opcode::BO_LOr;

                    Facts.ContainsConditionalEvaluation =
                        isInTree(ST, stmtIsA<clang::ConditionalOperator>()) ||
                        isInTree(ST, stmtIsBinOp(LogicalAnd)) ||
                        isInTree(ST, stmtIsBinOp(LogicalOr));

                    auto ArgStmts = collectStmtsFromArguments(Exp);

                    // Check if any subtree of the entire expansion
                    // that was not parsed from an argument is an expression
                    // whose type is a locally-defined type
                    Facts.ContainsSubExprFromBodyWithLocalOrAnonymousType =
                        isInTree(
                            ST,
                            [&ArgStmts](const clang::Stmt *St)
                            {
                                if (!St)
                                    return false;

                                if (ArgStmts.find(St) != ArgStmts.end())
                                    return false;

                                auto E = clang::dyn_cast<clang::Expr>(St);
                                if (!E)
                                    return false;

                                auto QT = E->getType();
                                if (QT.isNull())
                                    return false;

                                auto T = QT.getTypePtrOrNull();
                                if (!T)
                                    return false;

                                return T->hasUnnamedOrLocalType();
                            });

                    // Check if any variable or function this macro references
                    // that is not part of an argument was declared after this
                    // macro was defined
                    Facts.ContainsDeclFromBodyDefinedAfterMacro =
                        isInTree(
                            ST,
                            [&SM, &ArgStmts, DefLoc](const clang::Stmt *St)
                            {
                                if (!St)
                                    return false;

                                if (ArgStmts.find(St) != ArgStmts.end())
                                    return false;

                                auto DRE = clang::dyn_cast<clang::DeclRefExpr>(St);
                                if (!DRE)
                                    return false;

                                auto D = DRE->getDecl();
                                if (!D)
                                    return false;

                                auto L = D->getLocation();
                                if (L.isInvalid())
                                    return false;

                                L = SM.getFileLoc(L);
                                if (L.isInvalid())
                                    return false;

                                return SM.isBeforeInTranslationUnit(DefLoc, L);
                            });

                    // Check if any subtree of the entire expansion
                    // that was not parsed from an argument is an expression
                    // whose type is a type that was defined after the macro
                    // was defined
                    Facts.ContainsSubExprFromBodyWithTypeDefinedAfterMacro =
                        isInTree(
                            ST,
                            [&SM, &ArgStmts, DefLoc](const clang::Stmt *St)
                            {
                                if (!St)
                                    return false;

                                if (ArgStmts.find(St) != ArgStmts.end())
                                    return false;

                                auto E = clang::dyn_cast<clang::Expr>(St);
                                if (!E)
                                    return false;

                                auto QT = E->getType();

                                return containsTypeDefinedAfter(QT, SM, DefLoc);
                            });

                    Facts.TypeSignature = "void";
                    if (auto E = clang::dyn_cast<clang::Expr>(ST))
                    {
                        Facts.ASTKind = "Expr";

                        // Type information about the entire expansion
                        auto QT = E->getType();
                        auto T = QT.getTypePtrOrNull();
                        Facts.ExpansionHasType = !(QT.isNull() || T == nullptr);

                        if (T)
                        {
                            Facts.IsExpansionVoidType = T->isVoidType();
                            Facts.IsExpansionLocalOrAnonymousType = T->hasUnnamedOrLocalType();
                            auto CT = QT.getDesugaredType(Ctx).getUnqualifiedType().getCanonicalType();
                            Facts.TypeSignature = CT.getAsString();
                        }
                        Facts.ExpansionContainsTypeDefinedAfterMacroWasDefined =
                            containsTypeDefinedAfter(QT, SM, DefLoc);

                        // Whether the expression was expanded where a constant
                        // expression is required
                        Facts.ExpandedWhereConstExprRequired =
                            descendantOfConstantExpression(Ctx, E);

                        // Whether this expression is an integral
                        // constant expression
                        Facts.IsIntConstExpr = E->isIntegerConstantExpr(Ctx);
                    }
                }
                else if (D)
                    Facts.ASTKind = "Decl";
                else if (TL)
                {
                    debug("Aligns with a type loc");
                    Facts.ASTKind = "TypeLoc";
                    // Check that this type specifier list does not include
                    // a typedef that was defined after the macro was defined
                    auto QT = TL->getType();
                    Facts.IsNullType = QT.isNull();
                    debug("Checking TypeLocContainsTypeDefinedAfterMacroWasDefined");
                    Facts.TypeLocContainsTypeDefinedAfterMacroWasDefined =
                        containsTypeDefinedAfter(QT, SM, DefLoc);
                    debug("Finished checking TypeLocContainsTypeDefinedAfterMacroWasDefined");
                }
                else
                    assert("Aligns with node that is not a Decl/Stmt/TypeLoc");
            }

            Facts.NumArguments = Exp->Arguments.size();

            // Check that the number of AST nodes aligned with each argument
            // equals the number of times that argument was expanded
            debug("Checking for aligned arguments");
            Facts.HasAlignedArguments = std::all_of(
                Exp->Arguments.begin(),
                Exp->Arguments.end(),
                [](MacroExpansionArgument Arg)
                { return Arg.AlignedRoots.size() == Arg.NumExpansions; });

            if (Exp->MI->isFunctionLike() &&
                (Facts.ASTKind == "Stmt" || Facts.ASTKind == "Expr"))
                Facts.TypeSignature += "(";
            debug("Iterating arguments");
            int ArgNum = 0;
            for (auto &&Arg : Exp->Arguments)
            {
                if (ArgNum != 0)
                    Facts.TypeSignature += ", ";
                ArgNum += 1;

                Facts.HasUnexpandedArgument = Arg.AlignedRoots.empty();

                if (Arg.AlignedRoots.empty())
                    continue;

                auto Arg1stExpST = Arg.AlignedRoots.front().ST;
                auto E = clang::dyn_cast_or_null<clang::Expr>(Arg1stExpST);

                Facts.HasNonExprArgument = E == nullptr;

                if (!E)
                    continue;

                std::string ArgTypeStr = "<Null>";

                // Type information about arguments
                auto QT = E->getType();
                auto T = QT.getTypePtrOrNull();
                Facts.HasUntypedArgument = QT.isNull() || T == nullptr;

                if (T)
                {
                    Facts.HasArgumentWithVoidType = T->isVoidType();
                    Facts.HasArgumentWithLocalOrAnonymousType = T->hasUnnamedOrLocalType();
                    auto CT = QT.getDesugaredType(Ctx).getUnqualifiedType().getCanonicalType();
                    ArgTypeStr = CT.getAsString();
                }
                Facts.HasArgumentWithTypeDefinedAfterMacro =
                    containsTypeDefinedAfter(QT, SM, DefLoc);

                Facts.TypeSignature += ArgTypeStr;
            }
            debug("Finished iterating arguments");
            if (Exp->MI->isFunctionLike() &&
                (Facts.ASTKind == "Stmt" || Facts.ASTKind == "Expr"))
                Facts.TypeSignature += ")";

            // Check for semantic properties of interface-equivalence
            // TODO: Check for these properties in decls as well?
            //       Can decls ever be hygienic?
            debug("Checking if there is an aligned Stmt root");
            if (Exp->AlignedRoot && Exp->AlignedRoot->ST)
            {
                debug("Checking for semantic properties");
                Facts.IsHygienic = isHygienic(Exp);
                Facts.IsParameterSideEffectFree =
                    isParameterSideEffectFree(Exp);
                Facts.IsLValueIndependent = isLValueIndependent(Exp);
            }

            debug("Printing ", Exp->Name.str(), "facts");
            printFacts(Facts);
        }

        // Only delete top level expansions since deconstructor deletes
        // nested expansions
        debug("Deleting expansion nodes");
        for (auto &&Exp : MF->Expansions)
            if (Exp->Depth == 0)
                delete Exp;
    }
} // namespace cpp2c
