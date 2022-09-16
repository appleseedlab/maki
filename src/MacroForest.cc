#include "MacroForest.hh"

#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/IdentifierTable.h"
#include "clang/Lex/Token.h"
#include "clang/Lex/MacroArgs.h"
#include "clang/Lex/MacroInfo.h"

#include "llvm/Support/raw_ostream.h"

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

        // Initialize the new expansion with the parts we can get
        // directly from clang

        cpp2c::MacroExpansionNode *Expansion = new cpp2c::MacroExpansionNode();
        Expansion->Name = MacroNameTok.getIdentifierInfo()->getName();
        Expansion->DefinitionRange = clang::SourceRange(
            MD.getMacroInfo()->getDefinitionLoc(),
            MD.getMacroInfo()->getDefinitionEndLoc());
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
            auto MI = MD.getMacroInfo();

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
                MacroExpansionNode::Argument Arg;
                Arg.Name = (i < MI->getNumParams())
                               ? MI->params()[i]->getName()
                               : llvm::StringRef("__VA_ARGS__");

                // Collect the argument's token rages
                if (!ArgTokens.empty())
                {
                    // Iterate the rest of the argument's tokens,
                    // and merge their ranges if necessary to get the
                    // spelling ranges spanned by this argument's passed
                    // tokens.
                    // Normally the interval merge algorithm requires that
                    // the input intervals be first sorted by their starts
                    // and then ends, but we can skip this step since
                    // the input is inherently sorted this way
                    Arg.TokenRanges.push_back(
                        getSpellingRange(Ctx,
                                         ArgTokens.front().getLocation(),
                                         ArgTokens.front().getEndLoc()));
                    for (auto it = ArgTokens.begin() + 1;
                         it != ArgTokens.end();
                         it++)
                    {
                        auto CurrentRange = Arg.TokenRanges.back();
                        clang::SourceRange TokenRange =
                            getSpellingRange(Ctx,
                                             it->getLocation(),
                                             it->getEndLoc());
                        if (TokenRange.getBegin() <= CurrentRange.getEnd())
                            // Merge ranges
                            Arg.TokenRanges.back().setEnd(
                                CurrentRange.getEnd() > TokenRange.getEnd()
                                    ? CurrentRange.getEnd()
                                    : TokenRange.getEnd());
                        else
                            // Don't merge ranges
                            Arg.TokenRanges.push_back(TokenRange);
                    }
                }

                // Add the argument to the list of arguments for this expansion
                Expansion->Arguments.push_back(Arg);
            }
        }
    }

} // namespace cpp2c
