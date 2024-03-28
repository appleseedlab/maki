# Transformation Notes
Took about 3 hours in total

## ./ipc - transformed 48 / 49 macro definitions

### Macros not transformed
Did not transform the header guard macro _IPC_UTIL_H

### Macros transformed
name, portability levels, notes
MQUEUE_MAGIC, definition-adapting
DIRENT_SIZE, definition-adapting
FILENT_SIZE, definition-adapting
SEND, definition-adapting
RECV, definition-adapting
STATE_NONE, definition-adapting
STATE_READY, definition-adapting
SEARCH_ANY, definition-adapting
SEARCH_EQUAL, definition-adapting
SEARCH_NOTEQUAL, definition-adapting
SEARCH_LESSEQUAL, definition-adapting
SEARCH_NUMBER, definition-adapting
DATALEN_MSG, definition-adapting, added type-casts at call-sites to silence warnings but this was not strictly necessary
DATALEN_SEG, definition-adapting, added type-casts at call-sites to silence warnings but this was not strictly necessary
SEMMSL_FAST, definition-adapting
SEMOPM_FAST, definition-adapting
USE_GLOBAL_LOCK_HYSTERESIS, definition-adapting
msg_ids, calling-convention-adapting
sem_ids, calling-convention-adapting
sc_semmsl, calling-convention-adapting; call-site-context-altering, expanded to an array access of a struct array field
sc_semmns, calling-convention-adapting; call-site-context-altering, expanded to an array access of a struct array field
sc_semopm, calling-convention-adapting; call-site-context-altering, expanded to an array access of a struct array field
sc_semmni, calling-convention-adapting; call-site-context-altering, expanded to an array access of a struct array field
SEM_GLOBAL_LOCK, definition-adapting
SHM_DEST, definition-adapting
SHM_LOCKED, definition-adapting
shm_file_data, calling-convention-adapting, appears on the left- and right-hand-side of an assignment statement
shm_ids, calling-convention-adapting
shm_unlock, definition-adapting
COMPAT_SHMLBA (ipc/shm.c), definition-adapting, technically scope-adapting since its present in a CPP conditional, but this fact can be safely ignored in this case.
SIZE_SPEC, call-site-context-altering; expanded in sequence of strings meant to be concatenated together during translation phase 6. Transformed by lifting the entire concatenated string into the transformed macro. Also moved the definition to the top of the function to silence a warning, but this was not strictly necessary.
COMPAT_SHMLBA (ipc/syscall.c), definition-adapting, same as prior definition.
IPCMNI_SHIFT, definition-adapting
IPCMNI_EXTEND_SHIFT, definition-adapting
IPCMNI_EXTEND_MIN_CYCLE, definition-adapting
IPCMNI, definition-adapting
IPCMNI_EXTEND, definition-adapting
ipcmni_seq_shift, definition-adapting
IPCMNI_IDX_MASK, calling-convention-adapting? Had to turn an object-like macro into a function.
ipc_mni, definition-adapting
ipc_min_cycle, definition-adapting
ipc_init_proc_interface, definition-adapting
IPC_SEM_IDS, definition-adapting
IPC_MSG_IDS, definition-adapting
IPC_SHM_IDS, definition-adapting
ipcid_to_idx, definition-adapting, technically should not transform since this macro is never invoked; made a best-guess attempt at inferring function signature type
ipcid_to_seqx, definition-adapting, technically should not transform since this macro is never invoked; made a best-guess attempt at inferring function signature type
ipcid_seq_max, definition-adapting, technically should not transform since this macro is never invoked; made a best-guess attempt at inferring function signature type

# ./sound/atmel - transformed 54 / 55 macro definitions

### Macros not transformed
Header macro __SOUND_ATMEL_AC97C_H

### Macros transformed
name, portability levels, notes
get_chip, generic
ac97c_writel, metaprogramming (token-pasting), transformed by declaring an enum for each of the register names and using a switch statement to decide which one to return
ac97c_readl,  metaprogramming (token-pasting), transformed by declaring an enum for each of the register names and using a switch statement to decide which one to return
ATMEL_AC97C_PM_OPS, call-site-context-altering, Conditionally defined to the address of a global struct that was declared only under the same condition. This macro was then invoked in the initialization of a static global struct. I transformed this macro by duplicating and lifting the struct declaration it was invoked inside of into both branches of the static conditional the macro was defined under, and inlining its conditional definitions in both branches.
AC97C_MR, definition-adapting
AC97C_ICA, definition-adapting
AC97C_OCA, definition-adapting
AC97C_CARHR, definition-adapting
AC97C_CATHR, definition-adapting
AC97C_CASR, definition-adapting
AC97C_CAMR, definition-adapting
AC97C_CORHR, definition-adapting
AC97C_COTHR, definition-adapting
AC97C_COSR, definition-adapting
AC97C_COMR, definition-adapting
AC97C_SR, definition-adapting
AC97C_IER, definition-adapting
AC97C_IDR, definition-adapting
AC97C_IMR, definition-adapting
AC97C_VERSION, definition-adapting

AC97C_CATPR, definition-adapting? Unused and defined to a macro that is in turn undefined, so the simplest transformation was to remove it from the program
AC97C_CATCR, definition-adapting? Unused and defined to a macro that is in turn undefined, so the simplest transformation was to remove it from the program
AC97C_CATNPR, definition-adapting? Unused and defined to a macro that is in turn undefined, so the simplest transformation was to remove it from the program
AC97C_CATNCR, definition-adapting? Unused and defined to a macro that is in turn undefined, so the simplest transformation was to remove it from the program
AC97C_CARPR, definition-adapting? Unused and defined to a macro that is in turn undefined, so the simplest transformation was to remove it from the program
AC97C_CARCR, definition-adapting? Unused and defined to a macro that is in turn undefined, so the simplest transformation was to remove it from the program
AC97C_CARNPR, definition-adapting? Unused and defined to a macro that is in turn undefined, so the simplest transformation was to remove it from the program
AC97C_CARNCR, definition-adapting? Unused and defined to a macro that is in turn undefined, so the simplest transformation was to remove it from the program
AC97C_PTCR, definition-adapting? Unused and defined to a macro that is in turn undefined, so the simplest transformation was to remove it from the program

AC97C_MR_ENA, definition-adapting
AC97C_MR_WRST, definition-adapting
AC97C_MR_VRA, definition-adapting
AC97C_CSR_TXRDY, definition-adapting
AC97C_CSR_TXEMPTY, definition-adapting
AC97C_CSR_UNRUN, definition-adapting
AC97C_CSR_RXRDY, definition-adapting
AC97C_CSR_OVRUN, definition-adapting
AC97C_CSR_ENDTX, definition-adapting
AC97C_CSR_ENDRX, definition-adapting
AC97C_CMR_SIZE_20, definition-adapting
AC97C_CMR_SIZE_18, definition-adapting
AC97C_CMR_SIZE_16, definition-adapting
AC97C_CMR_SIZE_10, definition-adapting
AC97C_CMR_CEM_LITTLE, definition-adapting
AC97C_CMR_CEM_BIG, definition-adapting
AC97C_CMR_CENA, definition-adapting
AC97C_CMR_DMAEN, definition-adapting
AC97C_SR_CAEVT, definition-adapting
AC97C_SR_COEVT, definition-adapting
AC97C_SR_WKUP, definition-adapting
AC97C_SR_SOF, definition-adapting

AC97C_CH_MASK, metaprogramming (token-pasting), removed token-pasting and instead passed full tokens to macro invocations
AC97C_CH_ASSIGN, metaprogramming (token-pasting), removed token-pasting and instead passed full tokens to macro invocations
AC97C_CHANNEL_NONE, definition-adapting
AC97C_CHANNEL_A, definition-adapting