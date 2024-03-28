# Enscript Transformation Notes
- Transformation flow:
  - Tool Maki to print out a table of macros and their portability levels. Maki
    prints this output in sorted order such that macros that satisfy easier
    portability levels (e.g. `definition-altering` appears before
    `declaration-altering` appears before `call-site-context-altering`, etc.)
    appear first. This makes it easy to see which macros we should prioritize
    transforming first (ideally just go down the table's rows).
  - Transform each macro one at a time. After transforming a macro, run the
    program's test suite (e.g. `make check`) to verify that this transformation
    did not break anything. Then move on to the next transformation.

- Maki does not record data on generic macros
- Several macros (e.g. `CHECK_TOKEN()`, `FITS_ON_LINE()`) are found to be
  call-site-context-altering, but only because they expand to a `do-while`
- `MATCH()` and `EXISTS()` would be definition-altering, except that they are
  used generically
- `PRINT_BOUNDING_BOXES` is an example of a condition macro, but we can
  transform it by turning it into an if statement
