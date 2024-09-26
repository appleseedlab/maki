cmake_minimum_required(VERSION 3.29)
project(maki)

# ===============================================================================
# 1. VERIFY CLANG INSTALLATION DIR
# ===============================================================================

# Set Clang version.
set(MAKI_CLANG_VERSION 17)


# ===============================================================================
# 1. LOAD CLANG CONFIGURATION For more:
#   http://llvm.org/docs/CMake.html#embedding-llvm-in-your-project
# ===============================================================================


# Find Clang and LLVM.
find_package(Clang ${MAKI_CLANG_VERSION} REQUIRED CONFIG)
find_package(LLVM  REQUIRED CONFIG)

# Set this variable for configuring the test suite and wrapper script.
find_program(CLANG_C_COMPILER NAMES clang-${MAKI_CLANG_VERSION})

if (NOT CLANG_C_COMPILER)
  message(FATAL_ERROR "Clang executable not found")
endif()

message(STATUS "Found LLVM/Clang: ${LLVM_PACKAGE_VERSION}")

message(STATUS "CLANG STATUS:")
list(APPEND CMAKE_MESSAGE_INDENT "  ")
message(STATUS "Includes (clang)    ${CLANG_INCLUDE_DIRS}")
message(STATUS "Includes (llvm)     ${LLVM_INCLUDE_DIRS}")
list(POP_BACK CMAKE_MESSAGE_INDENT)


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
  add_subdirectory(test)
endif()
