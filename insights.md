total programs: 20

- generally, most macros expand to expressions, then stmts, then type locs, then decls
  - exceptions
    - enscript: more stmts than exprs
    - gzip: FAR more decls than anything else
    - mosaic and zsh: many more decls than exprs
    - zsh: more decls than type locs
      - this is funny because this is not true for the other shell, bash
        - good support for argument that developer preference, rather than application domain, determines how macros will be used
  - argument: most developers usually treat macro invocations like expressions

- perl has more invocations that expand to decls than any other program
  - partly because Perl is larger than any other program
  - almost all of these are object-like, e.g. DECLARATION_FOR_LC_NUMERIC_MANIPULATION, which is defined and invoked many times
    - i guess this helps perl devs standardize how they write code

  - the only program that has more invocations that expand to type locs than perl is gnuplot
  - perl has far more function-like macro invocations that expand to type locs than any other program
  - this indicates that perl code uses its own sort of template language
  - this inhibits our ability to transform perl without converting it to C++

- gzip has the second-most invocations that expand to decls, surpassed only by perl
  - unlike perl, all of gzip's invocations that expand to decls are function-like

The reason why gzip has so many decl invocations is because some of its macros are defined in multiple header files and invoked in header files.
For example, let's focus on _GL_CXXALIAS_SYS.
This macro is defined in at least five different header files.
It is also invoked in the header file lib/stdio.h.
When building gzip, lib/stdio.h is preprocessed multiple times, and so _GL_CXXALIAS_SYS is expanded multiple times.
Normally we would only consider the first expansion of _GL_CXXALIAS_SYS to be a unique invocation.
However, each time lib/stdio.h, _GL_CXXALIAS_SYS's definition comes from a different one of its five different definitions.
So each expansion is technically a unique invocation of a different macro definition, even if they all the expansions appear at the same source location.


Table X shows the distribution of the types of C AST subtrees that macros invocations align with.
For most programs, the majority of macro invocations do not align with an AST subtree, and the majority of aligned invocations align with expressions.
This means that if the primary reason that developers use macros is to break C syntax, then the second most common reason is to replace existing C syntax.
<!-- TODO: Point out exceptions to this rule? -->
This makes sense for a few reasons.
First, it is a convention in C to use object-like macros for global constants, instead of global `const` variables or enums.
Second, many C applications [CITE sqlite, lua, bash?] are compiled with a C compiler that does not support the `inline` keyword, so developers must use function-like macros instead of `inline` functions to optimize software performance.


- programs with more easy to transform macro defs than hard to transform ones:
  - bash - 2x more
  - flex - 0.5x more
  - fvwm - 1.8x more
  - gnuplot - a bit more
  - gv - a bit more
  - mosaic - a bit more
  - remind - 2x more
  - xfig - 9x more
  - zsh - 3x more
  - total: 10
    - bash, xfig and zsh have so many more easy to transform macros because they have lots of simple object-like macros
      - case in point: these programs have the most macro definitions that mennie can transform
      - also, if we only consider flms, then these programs stop having more easy to transform macros than hard ones


THIS IS GOOD
- programs with more easy to transform olm macro defs than hard to transform ones:
  - bash
  - emacs
  - enscript
  - flex
  - fvwm
  - gnuplot
  - gv
  - lua
  - m4
  - mosaic
  - rcs - equal
  - remind
  - xfig
  - zsh
  - total: 14

- programs with more easy to transform flm macro defs than hard to transform ones:
  - fvwm
  - total: 1
  - argument: object-like macro definitions are generally easier to transform

- harder to transform macros are invoked more often than easy to transform ones
  - argument: harder to transform macros represent abstractions of more complex logic, and abstractions are more desirable/useful for complex logic than simple logic
  - bash and enscript are the only programs to break this rule, and even only slightly, and only if we consider object-like macros



Table X compares the number of easy-to-transform to hard-to-transform macro definitions across all programs.
If we consider all macro definitions, then most programs had more hard-to-transform definitions than easy ones.
This follows from the fact that most macro invocations are unaligned, and thus require call-site-altering transformations to transform.
If we only consider object-like macros, then most programs have more easy-to-transform macro definitions than hard ones.
This is because most object-like macros expand to constants, which are trivial to transform into a variable or enum.
Finally, it is interesting to note that only one program, `fvwm`, had more easy-to-transform function-like macro definitions than hard ones.
This means that if developers are going to use macros to break C syntax, then they will probably do it using a function-like macro.
This may be because function-like macros are more versatile and powerful than object-like macros, so ... TODO

- even if we only consider interface-equivalent transformations, we always transform more invocations than mennie can
  - we can often transform 2x more invocations, sometimes even 5x more

- thunkizing transformations are the least common transformations
  - even then, may not be necessary in practice because all macros we examined only expand their arguments once, so side-effects won't be duplicated anyway
  - this makes sense: arguments with side-effects that are expanded multiple times is likely to be a bug
    - we couldn't find a single case of macro argument side-effect duplication
      - if we did, we probably would have found a bug
- again: developer preference is key

- what about property categories?
  - THIS IS GOOD
  - in order from most frequently satisfied to least for tlna invocations: syntactic, no properties, scoping, typing, calling convention, metaprogramming
    - this means developers most frequently use macros *because* they don't have to conform to C's AST
      - this suggests that if a C language construct already exists to accomplish a goal, devs are likely just use it instead of defining a macro
        - this seems reasonable
        - this also explains why most macros are hard to transform - if they were easy to transform, devs would have just used the transformed construct initially instead
      - the fact that it is next most common for developers to use macros that satisfy no properties undermines this claim though, because that suggests that developers often *do* use macros where a C construct would work just as well
        - this is more true for object-like macros - for flms, invocations that satisfy no properties are about as frequent as ones that satisfy only typing properties
        - this also makes sense because it is common C practice to use macros as global constants instead of enums
    - VERY INTERESTING
    - devs don't exploit calling convention differences between macros and function
      - in fact, they seem to try to avoid doing so
- interesting: invocations that satisfy typing properties also satisfy other properties more often than other macros
  - this is probably because a few typing properties are also scoping properties, so if a macro satisfies a typing property it may also satisfy a scoping property
  - note that this doesn't work in reverse: if a macro satisfies a scoping property, then that doesn't mean it's likely to satisfy a typing property
    - reason: there's many scoping properties that are also typing properties, but only a few typing properties that are also scoping properties

- Devs usually exploit macros for syntactic, scoping, or typing rules
