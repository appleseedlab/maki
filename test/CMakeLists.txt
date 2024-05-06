if(LLVM_EXTERNAL_LIT)
  get_filename_component(LLVM_LIT_TOOLS_DIR "${LLVM_EXTERNAL_LIT}" DIRECTORY)
  find_program(FILECHECK_PATH FileCheck HINTS "${LLVM_LIT_TOOLS_DIR}")
else()
  find_program(FILECHECK_PATH FileCheck)
endif()

if(FILECHECK_PATH STREQUAL "FILECHECK_PATH-NOTFOUND")
  message(FATAL_ERROR "Could not find FileCheck")
endif()

# Set this variable for configuring the test suite.
set(CLANG_C_COMPILER "clang-${MAKI_CLANG_VERSION}")

configure_lit_site_cfg(
  ${CMAKE_CURRENT_SOURCE_DIR}/lit.site.cfg.py.in
  ${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg.py
  MAIN_CONFIG
  ${CMAKE_CURRENT_SOURCE_DIR}/lit.cfg.py
)

set(MAKI_TEST_DEPENDS maki)

add_lit_testsuite(check-maki "Running maki regression tests"
  ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS ${MAKI_TEST_DEPENDS}
)

set_target_properties(check-maki PROPERTIES FOLDER "Tests")

add_lit_testsuites(maki ${CMAKE_CURRENT_SOURCE_DIR} DEPENDS ${MAKI_TEST_DEPENDS})
