# /**
#  * @file CMakeLists.txt
#  * @brief Top-level CMake configuration for SRRArch simulator
#  *
#  * Sets up C++17 standard, compiler warnings, output directories,
#  * and includes LLVM for potential future integration.
#  *
#  * @author SRRArch Simulator Team
#  * @version 0.1.0
#  * @date 2026
#  */

cmake_minimum_required(VERSION 3.15)
project(SRRArch-simulator VERSION 0.1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Include CMake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Find required packages
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Include directories
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# Set compiler options
include(CompilerOptions)

# Add subdirectories
add_subdirectory(src)

# Enable testing
enable_testing()