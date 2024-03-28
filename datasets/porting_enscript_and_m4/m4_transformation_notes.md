# Notes on Transforming Macros in M4
- Before modifying the m4's source code at all, when I configure and run `make
  check` on m4, I get the following initial output:
    ```
    Skipped checks were:
    ./125.changeword ./126.changeword ./127.changeword ./128.changeword ./129.changeword ./130.changeword
    Failed checks were:
    ./198.sysval:err
    ```
- Took about 15 minutes to transform all definition-altering macros in m4.
- Took about 4 hr to transform all other macros in m4 besides `DECLARE()`.

- Macros that are listed as call-site-context-altering because they contain
  conditional evaluation, and whose semantics wouldn't be affected by
  transforming them to a function call:
  - `ARG_STR()`
  - `ARG_INT()`

- Invoked on LHS of an assignment
  - `SYMBOL_TYPE()`

- False positive due to not being able to take the address of a bitfield:
  - `SYMBOL_DELETED()`
  - `SYMBOL_TRACED()`
  - `SYMBOL_MACRO_ARGS()`
  - `SYMBOL_BLIND_NO_ARGS()`
- Fix was to transform macro into two separate functions - a getter and a setter

- Used `va_list` to transform `M4ERROR()`'s definition into a function
  definition and used regular expressions to quickly transform all 96
  invocations into function calls

- Note: Multiple ways to transform `AUTHORS`
  - Could turn into a function. This is the approach I have opted to take.
  - Could turn into a const char *, but need to declare at non-global scope
    since initializer is non-const. This only works because this macro is only
    invoked once.
  

name, expected, actual, explanation
SYMBOL_DELETED, easy; calling-convention-adapting; scope-adapting, hard; call-site-context-altering, cannot take the address of a bitfield
SYMBOL_TRACED, easy; calling-convention-adapting; scope-adapting, hard; call-site-context-altering, cannot take the address of a bitfield
SYMBOL_MACRO_ARGS, easy; calling-convention-adapting; scope-adapting, hard; call-site-context-altering, cannot take the address of a bitfield
SYMBOL_BLIND_NO_ARGS, easy; calling-convention-adapting; scope-adapting, hard; call-site-context-altering, cannot take the address of a bitfield
AUTHORS, hard, easy; object-like macro that was initialized to a non-const value; needed to be turned into a function instead
STREQ, hard; generic, easy; definition-adapting, generic; can coerce arguments into being const
DEBUG_MESSAGE, hard; no portability levels, easy; definition-altering, not sure why Maki did not find this one. Maybe because its one argument (Fmt) was passed to a call to a variadic function? I tweaked the body of the definition to silence a warning; but this modification was not strictly necessary.
DEBUG_PRINT3, hard; no portability levels, easy; definition-altering, same as above
ARG_LONG, hard; no portability levels, easy; definition-altering, same as above
ARG_DOUBLE, hard; no portability levels, easy; definition-altering, same as above
DEBUG_MESSAGE2, easy; scope-adapting, hard; scope-adapting; generic, arguments were passed to a variadic function
DEBUG_PRINT1, easy; scope-adapting, hard; scope-adapting; generic, arguments were passed to a variadic function
