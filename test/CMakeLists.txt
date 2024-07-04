find_program(FILECHECK_PATH NAMES FileCheck FileCheck-${LLVM_VERSION_MAJOR})
message(STATUS "FileCheck found at ${FILECHECK_PATH}")
find_program(LIT_PATH NAMES lit llvm-lit)
message(STATUS "Lit found at ${LIT_PATH}")

if(NOT FILECHECK_PATH)
  message(FATAL_ERROR "Could not find FileCheck")
endif()

if(NOT LIT_PATH)
  message(FATAL_ERROR "Could not find LLVM LIT")
endif()

configure_file(lit.site.cfg.py.in lit.cfg.py @ONLY)

set(MAKI_TEST_DEPENDS maki)

add_custom_target(check-maki
  COMMAND ${LIT_PATH} -s ${CMAKE_CURRENT_BINARY_DIR} 
  DEPENDS ${MAKI_TEST_DEPENDS}
  COMMENT "Running maki tests"
  USES_TERMINAL
)
