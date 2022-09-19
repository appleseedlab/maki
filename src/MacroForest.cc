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

        // Initialize the new expansion with the parts we can get
        // directly from clang

        cpp2c::MacroExpansionNode *Expansion = new cpp2c::MacroExpansionNode();
        Expansion->Name = MacroNameTok.getIdentifierInfo()->getName();
        Expansion->MacroHash = MI->getDefinitionLoc().printToString(SM);
        Expansion->DefinitionRange = clang::SourceRange(
            MI->getDefinitionLoc(),
            MI->getDefinitionEndLoc());
        Expansion->SpellingRange = getSpellingRange(Ctx,
                                                    Range.getBegin(),
                                                    Range.getEnd());

        // Add the expansion to the forest

        clang::SourceLocation E = Expansion->SpellingRange.getEnd();
        // Pop elements until we either empty the stack or find the parent
        // of the current expansion
        while ((!InvocationStack.empty()) &&
               (!(InvocationStack.top()->DefinitionRange.fullyContains(E))))
            InvocationStack.pop();

        if (InvocationStack.empty())
        {
            // New root expansion
            Expansion->Depth = 0;
            TopLevelExpansions.push_back(Expansion);
        }
        else
        {
            // New child expansion
            Expansion->Parent = InvocationStack.top();
            Expansion->Parent->Children.push_back(Expansion);
            Expansion->Depth = Expansion->Parent->Depth + 1;
        }

        // Add this expansion to the stack
        InvocationStack.push(Expansion);

        if (Args != nullptr)
        {
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
                const auto &LO = Ctx.getLangOpts();
                for (auto Tok : MI->tokens())
                    if (clang::Lexer::getSpelling(Tok, SM, LO) == Arg.Name.str())
                        Arg.numberOfTimesExpanded++;

                // Add the argument to the list of arguments for this expansion
                Expansion->Arguments.push_back(Arg);
            }
        }
    }

} // namespace cpp2c
