# Add the Maki library.
add_library(maki SHARED)

# TODO(Brent): See if we can break up the maki library from its dependencies to
# reduce linking overhead. Also, isn't there a better way to specify what parts
# of Clang Maki depends on using target_link_libraries()?

# Specify Maki's sources as private.

# NOTE(Brent): If Maki is ever to be used as a sub-project, then these sources
# will likely need to be made public.
target_sources(maki
  PRIVATE
    ASTUtils.cc
    AlignmentMatchers.cc
    MakiAction.cc
    MakiASTConsumer.cc
    DefinitionInfoCollector.cc
    DeclStmtTypeLoc.cc
    DeclCollectorMatchHandler.cc
    ExpansionMatchHandler.cc
    IncludeCollector.cc
    MacroForest.cc
    MacroExpansionArgument.cc
    MacroExpansionNode.cc
    StmtCollectorMatchHandler.cc
    JSONPrinter.cc
)

# Include LLVM and Clang include directories as system headers to suppress
# warnings.
target_include_directories(maki SYSTEM
    PRIVATE
      ${LLVM_INCLUDE_DIRS}
      ${CLANG_INCLUDE_DIRS}
)

# Set compiler flags.
target_compile_options(maki PRIVATE
  -Wall
  -fdiagnostics-color=always
  # LLVM/Clang is normally built without RTTI. Be consistent with that.
  "$<IF:$<BOOL:${LLVM_ENABLE_RTTI}>,,-fno-rtti>"
)

# -fvisibility-inlines-hidden is set when building LLVM and on Darwin warnings
# are triggered if maki is built without this flag (though otherwise it builds
# fine). For consistency, add it here too.
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-fvisibility-inlines-hidden" SUPPORTS_FVISIBILITY_INLINES_HIDDEN_FLAG)
if (${SUPPORTS_FVISIBILITY_INLINES_HIDDEN_FLAG})
  target_compile_options(maki PRIVATE -fvisibility-inlines-hidden)
endif()

# Use C++17.
target_compile_features(maki PRIVATE cxx_std_17)
# Enforce that we must use C++17.
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Allow undefined symbols in shared objects on Darwin (this is the default
# behaviour on Linux).
target_link_libraries(maki "$<$<PLATFORM_ID:Darwin>:-undefined dynamic_lookup>")
