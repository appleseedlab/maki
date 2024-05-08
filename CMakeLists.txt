cmake_minimum_required(VERSION 3.29)
project(maki)

# ===============================================================================
# 1. VERIFY CLANG INSTALLATION DIR
# ===============================================================================

# Set Clang version.
set(MAKI_CLANG_VERSION 17)

# Set this to a valid Clang installation dir.
set(MAKI_LLVM_INSTALL_DIR
    "/usr"
    CACHE PATH "LLVM installation directory")

# Check that the LLVM include directory is readable.
set(MAKI_LLVM_INCLUDE_DIR
    "${MAKI_LLVM_INSTALL_DIR}/include/llvm-${MAKI_CLANG_VERSION}")
if(NOT IS_READABLE "${MAKI_LLVM_INCLUDE_DIR}")
  message(
    FATAL_ERROR "MAKI_LLVM_INSTALL_DIR (${MAKI_LLVM_INCLUDE_DIR}) is invalid.")
endif()

# Check that the Clang include directory is readable.
set(MAKI_LLVM_CMAKE_FILE
    "${MAKI_LLVM_INSTALL_DIR}/lib/cmake/clang-${MAKI_CLANG_VERSION}/ClangConfig.cmake"
)
if(NOT IS_READABLE "${MAKI_LLVM_CMAKE_FILE}")
  message(
    FATAL_ERROR "MAKI_LLVM_CMAKE_FILE (${MAKI_LLVM_CMAKE_FILE}) is invalid.")
endif()

# ===============================================================================
# 1. LOAD CLANG CONFIGURATION For more:
#   http://llvm.org/docs/CMake.html#embedding-llvm-in-your-project
# ===============================================================================

# Set LLVM and Clang installation directories so that find_package() can find
# them.
set(Clang_DIR "${MAKI_LLVM_INSTALL_DIR}/lib/cmake/clang-${MAKI_CLANG_VERSION}/")
set(LLVM_DIR "${MAKI_LLVM_INSTALL_DIR}/lib/cmake/llvm-${MAKI_CLANG_VERSION}/")

# Find Clang.
find_package(Clang REQUIRED CONFIG)

# Sanity check. As Clang does not expose e.g. `CLANG_VERSION_MAJOR` through
# AddClang.cmake, we have to use LLVM_VERSION_MAJOR instead. TODO: Revisit when
# next version is released.
if(NOT "${MAKI_CLANG_VERSION}" VERSION_EQUAL "${LLVM_VERSION_MAJOR}")
  message(
    FATAL_ERROR
      "Found LLVM ${LLVM_VERSION_MAJOR}, but need LLVM ${MAKI_CLANG_VERSION}")
endif()

message(STATUS "Found Clang ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using ClangConfig.cmake in: ${MAKI_LLVM_INSTALL_DIR}")

message(STATUS "CLANG STATUS:")
list(APPEND CMAKE_MESSAGE_INDENT "  ")
message(STATUS "Includes (clang)    ${CLANG_INCLUDE_DIRS}")
message(STATUS "Includes (llvm)     ${LLVM_INCLUDE_DIRS}")
list(POP_BACK CMAKE_MESSAGE_INDENT)

# Set this variable for configuring the test suite and wrapper script.
set(CLANG_C_COMPILER "clang-${MAKI_CLANG_VERSION}")

# ===============================================================================
# 1. maki BUILD CONFIGURATION
# ===============================================================================

# Set the build type to Debug if using a single-configuration generator and no
# build type is specified.

# Check Maki is the root project.
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  # Set the build type if not set already
  if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE
        Debug
        CACHE STRING "Build type (default Debug):" FORCE)
  endif()
endif()

# ===============================================================================
# 1. ADD SUB-TARGETS Doing this at the end so that all definitions and
#   link/include paths are available for the sub-projects.
# ===============================================================================

add_subdirectory(lib)
add_subdirectory(wrappers)

# Add test suite.
option(MAKI_ENABLE_TESTING "Controls whether to build Maki's test suite" OFF)
if(${MAKI_ENABLE_TESTING})
  enable_testing()
  # For LLVM lit test suite.
  include(AddLLVM)
  add_subdirectory(test)
endif()
