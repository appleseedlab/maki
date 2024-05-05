cmake_minimum_required(VERSION 3.13.4)
project(maki)

#===============================================================================
# 1. VERIFY CLANG INSTALLATION DIR
#===============================================================================
# Set this to a valid Clang installation dir
#set(MAKI_LLVM_INSTALL_DIR "" CACHE PATH "LLVM installation directory")
set(MAKI_LLVM_INSTALL_DIR "/usr")

# A bit of a sanity checking
set(MAKI_LLVM_INCLUDE_DIR "${MAKI_LLVM_INSTALL_DIR}/include/llvm-17")
if(NOT EXISTS "${MAKI_LLVM_INCLUDE_DIR}")
message(FATAL_ERROR
  " MAKI_LLVM_INSTALL_DIR (${MAKI_LLVM_INCLUDE_DIR}) is invalid.")
endif()

set(MAKI_LLVM_CMAKE_FILE "${MAKI_LLVM_INSTALL_DIR}/lib/cmake/clang-17/ClangConfig.cmake")
if(NOT EXISTS "${MAKI_LLVM_CMAKE_FILE}")
message(FATAL_ERROR
  " MAKI_LLVM_CMAKE_FILE (${MAKI_LLVM_CMAKE_FILE}) is invalid.")
endif()

#===============================================================================
# 2. LOAD CLANG CONFIGURATION
#    For more: http://llvm.org/docs/CMake.html#embedding-llvm-in-your-project
#===============================================================================
set(CLANG_VERSION 17)
list(APPEND CMAKE_PREFIX_PATH "${MAKI_LLVM_INSTALL_DIR}/lib/cmake/llvm-${CLANG_VERSION}/")
list(APPEND CMAKE_PREFIX_PATH "${MAKI_LLVM_INSTALL_DIR}/lib/cmake/clang-${CLANG_VERSION}/")

find_package(Clang REQUIRED CONFIG)

# Sanity check. As Clang does not expose e.g. `CLANG_VERSION_MAJOR` through
# AddClang.cmake, we have to use LLVM_VERSION_MAJOR instead.
# TODO: Revisit when next version is released.
if(NOT "${CLANG_VERSION}" VERSION_EQUAL "${LLVM_VERSION_MAJOR}")
  message(FATAL_ERROR "Found LLVM ${LLVM_VERSION_MAJOR}, but need LLVM ${CLANG_VERSION}")
endif()

set(CLANG_C_COMPILER "clang-${CLANG_VERSION}")
message(STATUS "Found Clang ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using ClangConfig.cmake in: ${MAKI_LLVM_INSTALL_DIR}")


message("CLANG STATUS:
  Includes (clang)    ${CLANG_INCLUDE_DIRS}
  Includes (llvm)     ${LLVM_INCLUDE_DIRS}"
)

# Set the LLVM and Clang header and library paths
include_directories(SYSTEM "${LLVM_INCLUDE_DIRS};${CLANG_INCLUDE_DIRS}")

#===============================================================================
# 3. maki BUILD CONFIGURATION
#===============================================================================
# Use the same C++ standard as LLVM does
set(CMAKE_CXX_STANDARD 17 CACHE STRING "")

# Build type
if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE
      STRING "Build type (default Debug):" FORCE)
endif()

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall\
    -fdiagnostics-color=always")
    
# LLVM/Clang is normally built without RTTI. Be consistent with that.
if(NOT LLVM_ENABLE_RTTI)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
endif()

# -fvisibility-inlines-hidden is set when building LLVM and on Darwin warnings
# are triggered if llvm-tutor is built without this flag (though otherwise it
# builds fine). For consistency, add it here too.
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-fvisibility-inlines-hidden" SUPPORTS_FVISIBILITY_INLINES_HIDDEN_FLAG)
if (${SUPPORTS_FVISIBILITY_INLINES_HIDDEN_FLAG} EQUAL "1")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility-inlines-hidden")
endif()

# Set the build directories
set(CMAKE_RUNTIME_OUTPUT_DIREMAKIORY "${PROJEMAKI_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIREMAKIORY "${PROJEMAKI_BINARY_DIR}/lib")

#===============================================================================
# 4. ADD SUB-TARGETS
# Doing this at the end so that all definitions and link/include paths are
# available for the sub-projects.
#===============================================================================
add_subdirectory(src)
add_subdirectory(wrappers)

option(MAKI_ENABLE_TESTING "Controls whether to build Maki's test suite" OFF)
if (${MAKI_ENABLE_TESTING})
  enable_testing()
  include(AddLLVM)          # For LLVM lit test suite.
  add_subdirectory(test)
endif()
