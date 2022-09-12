#include "MacroForest.hh"

#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/IdentifierTable.h"
#include "clang/Lex/Token.h"
#include "clang/Lex/MacroArgs.h"
#include "clang/Lex/MacroInfo.h"

#include "llvm/Support/raw_ostream.h"

namespace cpp2c
{

    MacroForest::MacroForest(clang::Preprocessor &PP, clang::ASTContext &Ctx)
        : PP(PP), Ctx(Ctx) {}

    void MacroForest::MacroExpands(const clang::Token &MacroNameTok,
                                   const clang::MacroDefinition &MD,
                                   clang::SourceRange Range,
                                   const clang::MacroArgs *Args)
    {

        // Initialize the new expansion with the parts we can get
        // directly from clang

        cpp2c::MacroExpansionNode *Expansion = new cpp2c::MacroExpansionNode();
        Expansion->Name = MacroNameTok.getIdentifierInfo()->getName().str();
        Expansion->DefinitionRange = clang::SourceRange(
            MD.getMacroInfo()->getDefinitionLoc(),
            MD.getMacroInfo()->getDefinitionEndLoc());
        Expansion->SpellingRange = clang::SourceRange(
            Ctx.getFullLoc(Range.getBegin()).getSpellingLoc(),
            Ctx.getFullLoc(Range.getEnd()).getSpellingLoc());

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

        // TODO: Make this more nuanced so that we can record which argument
        // each macro was expanded for
        // If this macro was expanded inside a macro argument, record that
        if (ArgumentOf != nullptr &&
            // Don't record arguments of arguments
            !(Expansion->Parent &&
              Expansion->Parent->ArgumentOf == ArgumentOf))
        {
            Expansion->ArgumentOf = ArgumentOf;
            ArgumentOf->MacroArgs.push_back(Expansion);
        }

        // Add this expansion to the stack
        InvocationStack.push(Expansion);

        if (Args != nullptr)
        {
            // Expand this expansion's arguments
            for (unsigned int i = 0; i < Args->getNumMacroArguments(); i++)
            {
                // Before visiting each argument, we backup the invocation
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

                // We also back up the current macro whose arguments we may
                // be expanding
                auto ArgumentOfCopy = ArgumentOf;
                ArgumentOf = Expansion;

                // This const_cast is ugly, but is fine
                const_cast<clang::MacroArgs *>(Args)->getPreExpArgument(i, PP);

                // After visiting each argument, restore the state
                InvocationStack = InvocationStackCopy;
                ArgumentOf = ArgumentOfCopy;
            }
        }
    }

} // namespace cpp2c
