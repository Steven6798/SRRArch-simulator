# Create libraries
add_library(utils_lib STATIC logger.cpp)
add_library(elf_lib STATIC elf_loader.cpp)
add_library(instruction_lib STATIC instruction.cpp)
add_library(registers_lib STATIC registers.cpp)
add_library(simulator_lib STATIC simulator.cpp memory.cpp)

# Specify include directories for all libraries
foreach(lib utils_lib elf_lib instruction_lib registers_lib simulator_lib)
    target_include_directories(${lib} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
endforeach()

# Define library dependencies
target_link_libraries(simulator_lib
    PUBLIC
    elf_lib
    instruction_lib
    registers_lib
)

# Base library dependencies for executables
set(BASE_LIBS
    simulator_lib
    utils_lib
    ${LLVM_LIBRARIES}
)

# Debug executable
add_executable(SRRArch-simulator-debug main.cpp)
target_compile_definitions(SRRArch-simulator-debug PRIVATE
    DEBUG
    CURRENT_LOG_LEVEL=LOG_LEVEL_INFO
)
target_compile_options(SRRArch-simulator-debug PRIVATE -g -O0)
target_link_libraries(SRRArch-simulator-debug PRIVATE ${BASE_LIBS})

# Release executable
add_executable(SRRArch-simulator-release main.cpp)
target_compile_definitions(SRRArch-simulator-release PRIVATE
    NDEBUG
    CURRENT_LOG_LEVEL=LOG_LEVEL_NONE
)
target_compile_options(SRRArch-simulator-release PRIVATE -O3 -flto)
target_link_options(SRRArch-simulator-release PRIVATE -flto)
target_link_libraries(SRRArch-simulator-release PRIVATE ${BASE_LIBS})

# Install targets
install(TARGETS SRRArch-simulator-debug SRRArch-simulator-release DESTINATION bin)