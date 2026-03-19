# /**
#  * @file CompilerOptions.cmake
#  * @brief Compiler warning and optimization settings
#  *
#  * Configures warning flags (-Wall, -Wextra, -Wconversion, etc.)
#  * and optimization levels for different build types
#  * (Debug, Release, RelWithDebInfo).
#  *
#  * @author SRRArch Simulator Team
#  * @version 0.1.0
#  * @date 2026
#  */

# Common compiler warnings for all builds
set(COMMON_WARNINGS
    -Wall
    -Wextra
    -Wpedantic
    -Wconversion
    -Wsign-conversion
    -Wshadow
    -Wnon-virtual-dtor
    -Wold-style-cast
    -Wcast-align
    -Wunused
    -Woverloaded-virtual
    -Wformat=2
)

# Apply warnings based on compiler
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    foreach(flag ${COMMON_WARNINGS})
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}")
    endforeach()
endif()