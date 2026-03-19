# Create libraries
add_library(utils_lib STATIC logger.cpp)
add_library(elf_lib STATIC elf_loader.cpp)
add_library(decoder_lib STATIC decoder.cpp instruction.cpp)
add_library(registers_lib STATIC registers.cpp)
add_library(simulator_lib STATIC simulator.cpp memory.cpp)

# Specify include directories for all libraries
target_include_directories(utils_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(elf_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(decoder_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(registers_lib PUBLIC ${CMAKE_CURRENT_Source_DIR})
target_include_directories(simulator_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Define library dependencies
target_link_libraries(simulator_lib
    PUBLIC
    elf_lib
    decoder_lib
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
target_compile_options(SRRArch-simulator-release PRIVATE -O3)
target_link_libraries(SRRArch-simulator-release PRIVATE ${BASE_LIBS})

# Install targets
install(TARGETS SRRArch-simulator-debug SRRArch-simulator-release DESTINATION bin)