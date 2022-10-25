#include "MacroForest.hh"

#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/IdentifierTable.h"
#include "clang/Lex/Token.h"
#include "clang/Lex/Lexer.h"
#include "clang/Lex/MacroArgs.h"
#include "clang/Lex/MacroInfo.h"

#include "llvm/Support/raw_ostream.h"

// TODO:    Check if we should treat expansions written in scratch space
//          differently from other expansions

namespace cpp2c
{

    inline clang::SourceRange getSpellingRange(clang::ASTContext &Ctx,
                                               clang::SourceLocation B,
                                               clang::SourceLocation E)
    {
        return clang::SourceRange(Ctx.getFullLoc(B).getSpellingLoc(),
                                  Ctx.getFullLoc(E).getSpellingLoc());
    }

    MacroForest::MacroForest(clang::Preprocessor &PP, clang::ASTContext &Ctx)
        : PP(PP), Ctx(Ctx) {}

    void MacroForest::MacroExpands(const clang::Token &MacroNameTok,
                                   const clang::MacroDefinition &MD,
                                   clang::SourceRange Range,
                                   const clang::MacroArgs *Args)
    {
        auto MI = MD.getMacroInfo();
        auto &SM = Ctx.getSourceManager();
        const auto &LO = Ctx.getLangOpts();

        // Initialize the new expansion with the parts we can get
        // directly from clang

        auto Expansion = new MacroExpansionNode();
        Expansion->MI = MD.getMacroInfo();
        Expansion->Name = MacroNameTok.getIdentifierInfo()->getName();
        Expansion->MacroHash = MI->getDefinitionLoc().printToString(SM);
        Expansion->DefinitionRange = clang::SourceRange(
            MI->getDefinitionLoc(),
            MI->getDefinitionEndLoc());
        Expansion->DefinitionTokens = MI->tokens();
        Expansion->SpellingRange = getSpellingRange(Ctx,
                                                    Range.getBegin(),
                                                    Range.getEnd());
        Expansion->InMacroArg = InMacroArg;

        // Add the expansion to the forest

        clang::SourceLocation E = Expansion->SpellingRange.getEnd();
        // Pop elements until we either empty the stack or find the parent
        // of the current expansion
        while ((!InvocationStack.empty()) &&
               (!(InvocationStack.top()->DefinitionRange.fullyContains(E))))
            InvocationStack.pop();

        if (InvocationStack.empty())
            // New root expansion
            Expansion->Depth = 0;
        else
        {
            // New child expansion
            Expansion->Parent = InvocationStack.top();
            Expansion->Parent->Children.push_back(Expansion);
            Expansion->Depth = Expansion->Parent->Depth + 1;
        }
        Expansions.push_back(Expansion);

        // Add this expansion to the stack
        InvocationStack.push(Expansion);

        if (Args != nullptr)
        {
            // Save whatever the state of being in a macro argument is
            // before iterating arguments
            bool InMacroArgBefore = InMacroArg;
            InMacroArg = true;
            // Expand this expansion's arguments
            for (unsigned int i = 0; i < Args->getNumMacroArguments(); i++)
            {
                // Before expanding each argument, we backup the invocation
                // stack, clear it, and add the current invocation's
                // parent to it.
                // We do this in order to maintain our invariant that the
                // invocation stack only ever contain the parent or prior
                // siblings of the current invocation.
                // If we did not clear the stack between arguments, then
                // the stack might contain invocations nested under a previous
                // sibling argument, which would violate our invariant.
                auto InvocationStackCopy = InvocationStack;
                while (!InvocationStack.empty())
                    InvocationStack.pop();
                // Only push parent if non-null
                if (Expansion->Parent)
                    InvocationStack.push(Expansion->Parent);

                // This const_cast is ugly, but is fine
                auto ArgTokens = const_cast<clang::MacroArgs *>(Args)
                                     ->getPreExpArgument(i, PP);

                // After expanding each argument, restore the state
                InvocationStack = InvocationStackCopy;

                // Construct the next argument to add to the invocation's
                // argument list
                MacroExpansionArgument Arg;
                Arg.Name = (i < MI->getNumParams())
                               ? MI->params()[i]->getName()
                               : llvm::StringRef("__VA_ARGS__");

                // Collect the argument's tokens
                if (!ArgTokens.empty())
                {
                    Arg.Tokens = ArgTokens;
                    // Remove the last token since it will always be the EOF
                    // token for this argument
                    Arg.Tokens.pop_back();
                }

                // Count how many times this argument is expanded in
                // the macro body
                for (auto Tk : MI->tokens())
                    if (clang::Lexer::getSpelling(Tk, SM, LO) == Arg.Name.str())
                        Arg.NumExpansions++;

                // Add the argument to the list of arguments for this expansion
                Expansion->Arguments.push_back(Arg);
            }
            // Restore state of being in a macro argument
            InMacroArg = InMacroArgBefore;
        }

        if (!MI->tokens_empty())
        {
            // Check if the macro definition begins or ends with an argument
            for (auto &&Arg : Expansion->Arguments)
            {
                if (clang::Lexer::getSpelling(
                        MI->tokens().front(), SM, LO) == Arg.Name.str())
                    Expansion->ArgDefBeginsWith = &Arg;
                if (clang::Lexer::getSpelling(
                        MI->tokens().back(), SM, LO) == Arg.Name.str())
                    Expansion->ArgDefEndsWith = &Arg;
            }

            // Check if the macro performs stringification or token-pasting
            for (auto &&Tok : MI->tokens())
                if (Tok.is(clang::tok::TokenKind::hash))
                    Expansion->HasStringification = true;
                else if (Tok.is(clang::tok::TokenKind::hashhash))
                    Expansion->HasTokenPasting = true;
        }

        // Update the status of the expansion's parent as well
        if (auto P = Expansion->Parent)
        {
            P->HasStringification |= Expansion->HasStringification;
            P->HasTokenPasting |= Expansion->HasTokenPasting;
        }
    }

} // namespace cpp2c
