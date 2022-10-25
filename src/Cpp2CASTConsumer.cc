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

// TODO:    Collect names of macros which appear in #ifdefs and #undefs

// NOTE:    We can't use TK_IgnoreUnlessSpelledInSource because it ignores
//          paren exprs

namespace cpp2c
{
    static const constexpr char *delim = "\t";
    static const constexpr char *argsDelim = "++++";
    inline std::string fmt(std::string s) { return s; }
    inline std::string fmt(const char *s) { return std::string(s); }
    inline std::string fmt(bool b) { return b ? "True" : "False"; }
    inline std::string fmt(unsigned int i) { return std::to_string(i); }

    template <typename T>
    inline void print(T t, const char *end = delim)
    {
        llvm::outs() << fmt(t) << end;
    }

    inline void delimit() { print(delim, ""); }

    struct MacroFacts
    {
        std::string
            Name,
            DefLocOrError,
            InvokeLocOrError,
            ASTKind;

        unsigned int
            Depth,
            NumASTRoots;

        bool
            // All macros
            IsObjectLike,
            InMacroArg,
            ValidDefLoc,
            ValidInvokeLoc,
            HasStringification,
            HasTokenPasting,
            InvokesLaterDefinedMacro,
            // InStaticConditional,
            // InUndef,

            // Stmt/Expr macros
            ContainsDeclRefExpr,
            ContainsConditionalEvaluation,
            ContainsSubExprFromBodyWithLocallyDefinedType,
            ContainsDeclFromBodyDefinedAfterMacro,
            ContainsSubExprFromBodyWithTypeDefinedAfterMacro,
            ContainsSubExprFromBodyWithTypedefTypeDefinedAfterMacro,
            ExpansionHasType,

            // Semantic properties for Stmt/Expr macros
            IsHygienic,
            IsParameterSideEffectFree,
            IsLValueIndependent,

            // Expr macros
            IsExpansionVoidType,
            IsExpansionLocallyDefinedType,
            IsExpansionAnonymousType,
            ExpansionContainsTypeDefinedAfterMacroWasDefined,
            ExpansionContainsTypedefTypeDefinedAfterMacroWasDefined,
            ExpandedWhereConstExprRequired,

            // TypeLoc macros
            IsNullType,
            ContainsTypedefDefinedAfterMacroWasDefined,

            // Macro arguments
            HasArguments,
            HasAlignedArguments,
            HasUnexpandedArgument,
            HasNonExprArgument,
            HasUntypedArgument,
            HasArgumentWithVoidType,
            HasArgumentWithLocallyDefinedType,
            HasArgumentWithAnonymousType,
            HasArgumentWithTypeDefinedAfterMacro,
            HasArgumentWithTypedefTypeDefinedAfterMacro;
    };

    void printFacts(struct MacroFacts F)
    {
        print("Invocation");

        print(F.Name);
        print(F.DefLocOrError);
        print(F.InvokeLocOrError);

        print(F.Depth);

        print(F.IsObjectLike);
        print(F.InMacroArg);
        print(F.ValidDefLoc);
        print(F.ValidInvokeLoc);
        print(F.HasStringification);
        print(F.HasTokenPasting);
        print(F.InvokesLaterDefinedMacro, "");

        if (F.Depth != 0 || F.InMacroArg)
        {
            print("\n", "");
            return;
        }
        else
            delimit();

        print(F.NumASTRoots, "");

        if (F.NumASTRoots == 1)
        {
            delimit();

            print(F.ASTKind, "");
            if (F.ASTKind == "Stmt" || F.ASTKind == "Expr")
            {
                delimit();

                print(F.ContainsDeclRefExpr);
                print(F.ContainsConditionalEvaluation);
                print(F.ContainsSubExprFromBodyWithLocallyDefinedType);
                print(F.ContainsDeclFromBodyDefinedAfterMacro);
                print(F.ContainsSubExprFromBodyWithTypeDefinedAfterMacro);
                print(F.ContainsSubExprFromBodyWithTypedefTypeDefinedAfterMacro);

                print(F.IsHygienic);
                print(F.IsParameterSideEffectFree);
                print(F.IsLValueIndependent);

                print(F.ExpansionHasType, "");

                if (F.ASTKind == "Expr" && F.ExpansionHasType)
                {
                    delimit();

                    print(F.IsExpansionVoidType);
                    print(F.IsExpansionLocallyDefinedType);
                    print(F.IsExpansionAnonymousType);
                    print(F.ExpansionContainsTypeDefinedAfterMacroWasDefined);
                    print(F.ExpansionContainsTypedefTypeDefinedAfterMacroWasDefined);
                    print(F.ExpandedWhereConstExprRequired, "");
                }
            }
            else if (F.ASTKind == "Decl")
            {
                // Do nothing
            }
            else if (F.ASTKind == "TypeLoc")
            {
                delimit();

                print(F.IsNullType);
                print(F.ContainsTypedefDefinedAfterMacroWasDefined, "");
            }
        }

        // Separate body facts from argument facts
        print(argsDelim, "");

        print(F.HasArguments, "");
        if (F.HasArguments)
        {
            delimit();

            print(F.HasAlignedArguments);
            print(F.HasUnexpandedArgument);
            print(F.HasNonExprArgument);
            print(F.HasUntypedArgument);
            print(F.HasArgumentWithVoidType);
            print(F.HasArgumentWithLocallyDefinedType);
            print(F.HasArgumentWithAnonymousType);
            print(F.HasArgumentWithTypeDefinedAfterMacro);
            print(F.HasArgumentWithTypedefTypeDefinedAfterMacro, "");
        }

        print("\n", "");
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
                if (auto T = VD->getType().getTypePtr())
                    if (T->isArrayType())
                        return true;

            for (auto P : Ctx.getParents(Cur))
                Q.push(P);
        }
        return false;
    }

    // Returns a set of all Stmts parsed from the given expansion's arguments
    std::set<const clang::Stmt *>
    collectStmtsFromArguments(MacroExpansionNode *Expansion)
    {
        std::set<const clang::Stmt *> Result;
        for (auto &&Arg : Expansion->Arguments)
            for (auto &&R : Arg.AlignedRoots)
            {
                if (R.ST)
                {
                    std::queue<const clang::Stmt *> Q;
                    Q.push(R.ST);
                    while (!Q.empty())
                    {
                        auto Cur = Q.front();
                        Q.pop();
                        Result.insert(Cur);
                        for (auto &&Child : Cur->children())
                            Q.push(Child);
                    }
                }
            }
        return Result;
    }

    // Returns true if the given predicate returns true for any type
    // contained in the given type
    inline bool isInType(
        const clang::Type *T,
        std::function<bool(const clang::Type *)> pred)
    {
        assert(T);
        const clang::Type *Cur = T;
        while (Cur)
            if (pred(Cur))
                return true;
            else if (Cur->isAnyPointerType() || Cur->isArrayType())
                Cur = Cur->getPointeeOrArrayElementType();
            else
                Cur = nullptr;
        return false;
    }

    // Returns true if the given type contains an anonymous type
    inline bool containsAnonymousType(const clang::Type *T)
    {
        return isInType(
            T,
            [](const clang::Type *T)
            { return T->getAsTagDecl() &&
                     T->getAsTagDecl()->getName().empty(); });
    }

    // Returns true if the given type contains a locally-defined type
    inline bool containsLocalType(const clang::Type *T)
    {
        return isInType(
            T,
            [](const clang::Type *T)
            {
                return T->getAsTagDecl() &&
                       ((!T->getAsTagDecl()->getDeclContext()) ||
                        !(T->getAsTagDecl()
                              ->getDeclContext()
                              ->isTranslationUnit()));
            });
    }

    // Returns true if any type in T was defined after L
    inline bool containsTypeDefinedAfter(
        const clang::Type *T,
        clang::SourceManager &SM,
        clang::SourceLocation L)
    {
        return isInType(
            T,
            [&SM, &L](const clang::Type *T)
            {
                return T->getAsTagDecl() &&
                       SM.isBeforeInTranslationUnit(
                           L,
                           SM.getFileLoc(T->getAsTagDecl()->getLocation()));
            });
    }

    // Returns true if any typedef in T was defined after L
    inline bool containsTypedefDefinedAfter(
        const clang::Type *T,
        clang::SourceManager &SM,
        clang::SourceLocation L)
    {
        return isInType(
            T,
            [&SM, &L](const clang::Type *T)
            {
                return clang::isa<clang::TypedefType>(T) &&
                       clang::dyn_cast<clang::TypedefType>(T)->getDecl() &&
                       SM.isBeforeInTranslationUnit(
                           L,
                           SM.getFileLoc(
                               clang::dyn_cast<clang::TypedefType>(T)
                                   ->getDecl()
                                   ->getLocation()));
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

        DefLocs.push_back(SM.getFileLoc(Expansion->MI->getDefinitionLoc()));

        for (auto &&Child : Expansion->Children)
            collectExpansionDefLocs(SM, DefLocs, Child);
    }

    std::pair<bool, std::string> tryGetFullSourceLoc(
        clang::SourceManager &SM,
        clang::SourceLocation L)
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
                    return {false, "Invalid SLoc"};
                }
                return {false, "Nameless file"};
            }
            return {false, "File without FileEntry"};
        }
        return {false, "Invalid file ID"};
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

        // Check that the included file actually has a name
        auto IncludedFileRealpath = FE->tryGetRealPathName();
        if (IncludedFileRealpath.empty())
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
        if (LocalIncludes.find(IncludedInRealpath) !=
            LocalIncludes.end())
            return {false, IncludedFileRealpath};

        // Check that the include does not appear within the range of any
        // declaration in the file
        auto L = SM.getFileLoc(IEL.second);
        if (std::any_of(
                Decls.begin(),
                Decls.end(),
                [&SM, &LO, &L](const clang::Decl *D)
                {
                    auto B = SM.getFileLoc(D->getBeginLoc());
                    auto E = SM.getFileLoc(D->getEndLoc());

                    // Include the location just after the declaration
                    // to account for semicolons.
                    // If the decl does not have semicolon after it,
                    // that's fine since it would be a non-global
                    // location anyway
                    if (auto Tok = clang::Lexer::findNextToken(E, SM, LO))
                        if (Tok.hasValue())
                            E = SM.getFileLoc(Tok.getValue().getEndLoc());

                    return clang::SourceRange(B, E).fullyContains(L);
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
        DC = new cpp2c::DefinitionInfoCollector();

        PP.addPPCallbacks(std::unique_ptr<cpp2c::MacroForest>(MF));
        PP.addPPCallbacks(std::unique_ptr<cpp2c::IncludeCollector>(IC));
        PP.addPPCallbacks(std::unique_ptr<cpp2c::DefinitionInfoCollector>(DC));
    }

    void Cpp2CASTConsumer::HandleTranslationUnit(clang::ASTContext &Ctx)
    {
        auto &SM = Ctx.getSourceManager();
        auto &LO = Ctx.getLangOpts();

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

        // Print include-directive information
        {
            std::set<llvm::StringRef> LocalIncludes;
            for (auto &&IEL : IC->IncludeEntriesLocs)
            {
                // Facts for includes
                bool Valid;
                std::string IncludeName;

                // Check if included at global scope or not
                auto Res = isGlobalInclude(SM, LO, IEL, LocalIncludes, Decls);
                if (!Res.first && !Res.second.empty())
                    LocalIncludes.insert(Res.second);

                Valid = Res.first;
                IncludeName = Res.second.empty() ? "None" : Res.second.str();

                print("Include");
                print(Valid);
                print(IncludeName);

                llvm::outs() << "\n";
            }
        }

        // Print macro expansion information

        for (auto Exp : MF->Expansions)
        {
            //// First get basic info about all macros
            struct MacroFacts Facts = (struct MacroFacts){
                .Name = Exp->Name.str(),

                .Depth = Exp->Depth,

                .IsObjectLike = Exp->MI->isObjectLike(),
                .InMacroArg = Exp->InMacroArg,
                .HasStringification = Exp->HasStringification,
                .HasTokenPasting = Exp->HasTokenPasting,

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
                    [&SM, &DefLoc](clang::SourceLocation L)
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
            // Match stmts
            if (!(Exp)->DefinitionTokens.empty())
            {
                MatchFinder Finder;
                ExpansionMatchHandler Handler;
                auto Matcher = stmt(unless(anyOf(implicitCastExpr(),
                                                 implicitValueInitExpr())),
                                    alignsWithExpansion(&Ctx, Exp))
                                   .bind("root");
                Finder.addMatcher(Matcher, &Handler);
                Finder.matchAST(Ctx);
                for (auto M : Handler.Matches)
                    Exp->ASTRoots.push_back(M);
            }

            // Match decls
            if (!(Exp->DefinitionTokens.empty()))
            {
                MatchFinder Finder;
                ExpansionMatchHandler Handler;
                auto Matcher = decl(alignsWithExpansion(&Ctx, Exp))
                                   .bind("root");
                Finder.addMatcher(Matcher, &Handler);
                Finder.matchAST(Ctx);
                for (auto M : Handler.Matches)
                    Exp->ASTRoots.push_back(M);
            }

            // Match type locs
            if (!((Exp)->DefinitionTokens.empty()))
            {
                MatchFinder Finder;
                ExpansionMatchHandler Handler;
                auto Matcher = typeLoc(alignsWithExpansion(&Ctx, (Exp)))
                                   .bind("root");
                Finder.addMatcher(Matcher, &Handler);
                Finder.matchAST(Ctx);
                for (auto M : Handler.Matches)
                    Exp->ASTRoots.push_back(M);
            }

            // If the expansion only aligns with one node, then set this
            // as its aligned root
            Exp->AlignedRoot = (Exp->ASTRoots.size() == 1)
                                   ? (&(Exp->ASTRoots.front()))
                                   : nullptr;

            //// Find AST roots aligned with each of the expansion's arguments

            for (auto &&Arg : Exp->Arguments)
            {
                // Match stmts
                if (!Arg.Tokens.empty())
                {
                    MatchFinder Finder;
                    ExpansionMatchHandler Handler;
                    auto Matcher = stmt(unless(anyOf(implicitCastExpr(),
                                                     implicitValueInitExpr())),
                                        isSpelledFromTokens(&Ctx, Arg.Tokens))
                                       .bind("root");
                    Finder.addMatcher(Matcher, &Handler);
                    Finder.matchAST(Ctx);
                    for (auto M : Handler.Matches)
                        Arg.AlignedRoots.push_back(M);
                }

                // Match decls
                if (!(Arg.Tokens.empty()))
                {
                    MatchFinder Finder;
                    ExpansionMatchHandler Handler;
                    auto Matcher = decl(isSpelledFromTokens(&Ctx, Arg.Tokens))
                                       .bind("root");
                    Finder.addMatcher(Matcher, &Handler);
                    Finder.matchAST(Ctx);
                    for (auto M : Handler.Matches)
                        Arg.AlignedRoots.push_back(M);
                }

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
                    for (auto M : Handler.Matches)
                        Arg.AlignedRoots.push_back(M);
                }
            }

            //// Print macro info

            // Exp->dumpMacroInfo(llvm::outs());

            // Exp->dumpASTInfo(llvm::outs(),
            //                  Ctx.getSourceManager(), Ctx.getLangOpts());

            // Number of AST roots
            Facts.NumASTRoots = Exp->ASTRoots.size();

            if (Exp->ASTRoots.size() == 1)
            {
                auto D = Exp->AlignedRoot->D;
                auto ST = Exp->AlignedRoot->ST;
                auto TL = Exp->AlignedRoot->TL;

                if (ST)
                {
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
                    Facts.ContainsSubExprFromBodyWithLocallyDefinedType =
                        isInTree(
                            ST,
                            [&ArgStmts](const clang::Stmt *ST)
                            {
                                if (ArgStmts.find(ST) == ArgStmts.end())
                                    if (auto E =
                                            clang::dyn_cast<clang::Expr>(ST))
                                        if (auto T = E->getType()
                                                         .getTypePtrOrNull())
                                            return containsLocalType(T);
                                return false;
                            });

                    // Check if any variable or function this macro references
                    // that is not part of an argument was declared after this
                    // macro was defined
                    Facts.ContainsDeclFromBodyDefinedAfterMacro =
                        isInTree(
                            ST,
                            [&SM, &ArgStmts, DefLoc](const clang::Stmt *ST)
                            {
                                if (ArgStmts.find(ST) == ArgStmts.end())
                                    if (auto DRE =
                                            clang::dyn_cast<
                                                clang::DeclRefExpr>(ST))
                                        if (auto D = DRE->getDecl())
                                            return SM.isBeforeInTranslationUnit(
                                                DefLoc,
                                                SM.getFileLoc(D->getLocation()));
                                return false;
                            });

                    // Check if any subtree of the entire expansion
                    // that was not parsed from an argument is an expression
                    // whose type is a type that was defined after the macro
                    // was defined
                    Facts.ContainsSubExprFromBodyWithTypeDefinedAfterMacro =
                        isInTree(
                            ST,
                            [&SM, &ArgStmts, &DefLoc](const clang::Stmt *ST)
                            {
                                if (ArgStmts.find(ST) == ArgStmts.end())
                                    if (auto E =
                                            clang::dyn_cast<clang::Expr>(ST))
                                        if (auto T =
                                                E->getType().getTypePtrOrNull())
                                            return containsTypeDefinedAfter(
                                                T, SM, DefLoc);
                                return false;
                            });

                    // Check if any subtree of the entire expansion
                    // that was not parsed from an argument is an expression
                    // whose type is a typedef type that was defined after the
                    // macro was defined
                    Facts.ContainsSubExprFromBodyWithTypedefTypeDefinedAfterMacro =
                        isInTree(
                            ST,
                            [&SM, &ArgStmts, &DefLoc](const clang::Stmt *ST)
                            {
                                if (ArgStmts.find(ST) == ArgStmts.end())
                                    if (auto E =
                                            clang::dyn_cast<clang::Expr>(ST))
                                        if (auto T =
                                                E->getType().getTypePtrOrNull())
                                            return containsTypedefDefinedAfter(
                                                T, SM, DefLoc);
                                return false;
                            });

                    if (auto E = clang::dyn_cast<clang::Expr>(ST))
                    {
                        Facts.ASTKind = "Expr";
                        auto T = E->getType().getTypePtrOrNull();
                        Facts.ExpansionHasType = T != nullptr;
                        // Type information about the entire expansion
                        if (T)
                        {
                            Facts.ExpansionHasType = true;
                            Facts.IsExpansionVoidType = T->isVoidType();
                            Facts.IsExpansionLocallyDefinedType = containsLocalType(T);
                            Facts.IsExpansionAnonymousType = containsAnonymousType(T);
                            Facts.ExpansionContainsTypeDefinedAfterMacroWasDefined =
                                containsTypeDefinedAfter(T, SM, DefLoc);
                            Facts.ExpansionContainsTypedefTypeDefinedAfterMacroWasDefined =
                                containsTypedefDefinedAfter(T, SM, DefLoc);
                        }

                        // Whether the expression was expanded where a constant
                        // expression is required
                        Facts.ExpandedWhereConstExprRequired =
                            descendantOfConstantExpression(Ctx, E);
                    }
                }
                else if (D)
                    Facts.ASTKind = "Decl";
                else if (TL)
                {
                    Facts.ASTKind = "TypeLoc";
                    // Check that this type specifier list does not include
                    // a typedef that was defined after the macro was defined
                    if (auto T = TL->getTypePtr())
                        Facts.ContainsTypedefDefinedAfterMacroWasDefined =
                            containsTypedefDefinedAfter(T, SM, DefLoc);
                    else
                        Facts.IsNullType = true;
                }
                else
                    assert("Aligns with node that is not a Decl/Stmt/TypeLoc");
            }

            Facts.HasArguments = !Exp->Arguments.empty();

            // Check that the number of AST nodes aligned with each argument
            // equals the number of times that argument was expanded
            Facts.HasAlignedArguments = std::all_of(
                Exp->Arguments.begin(),
                Exp->Arguments.end(),
                [](MacroExpansionArgument Arg)
                { return Arg.AlignedRoots.size() == Arg.NumExpansions; });

            for (auto &&Arg : Exp->Arguments)
            {
                Facts.HasUnexpandedArgument = Arg.AlignedRoots.empty();

                if (Arg.AlignedRoots.empty())
                    continue;

                auto Arg1stExpST = Arg.AlignedRoots.front().ST;
                auto E = clang::dyn_cast_or_null<clang::Expr>(Arg1stExpST);

                Facts.HasNonExprArgument = E == nullptr;

                if (!E)
                    continue;

                auto T = E->getType().getTypePtrOrNull();

                Facts.HasUntypedArgument = T == nullptr;

                if (!T)
                    continue;

                // Type information about arguments
                Facts.HasArgumentWithVoidType = T->isVoidType();
                Facts.HasArgumentWithLocallyDefinedType = containsLocalType(T);
                Facts.HasArgumentWithAnonymousType = containsAnonymousType(T);
                Facts.HasArgumentWithTypeDefinedAfterMacro =
                    containsTypeDefinedAfter(T, SM, DefLoc);
                Facts.HasArgumentWithTypedefTypeDefinedAfterMacro =
                    containsTypedefDefinedAfter(T, SM, DefLoc);
            }

            // Check for semantic properties of interface-equivalence
            // TODO: Check for these properties in decls as well?
            //       Can decls ever be hygienic?
            if (Exp->AlignedRoot && Exp->AlignedRoot->ST)
            {
                Facts.IsHygienic = isHygienic(Ctx, Exp);
                Facts.IsParameterSideEffectFree =
                    isParameterSideEffectFree(Ctx, Exp);
                Facts.IsLValueIndependent = isLValueIndependent(Ctx, Exp);
            }

            printFacts(Facts);
        }

        // Free top level expansions since the deconstructor will free
        // nested expansions
        for (auto &&Exp : MF->Expansions)
            if (Exp->Depth == 0)
                delete Exp;
    }
} // namespace cpp2c
