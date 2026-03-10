# Main executable
add_executable(SRRArch-simulator main.cpp)

# Create libraries
add_library(utils_lib logger.cpp)
add_library(elf_lib elf_loader.cpp)
add_library(decoder_lib decoder.cpp)
add_library(registers_lib registers.cpp)
add_library(simulator_lib simulator.cpp memory.cpp)

# Specify include directories for all libraries
target_include_directories(utils_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(elf_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(decoder_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(registers_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(simulator_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(SRRArch-simulator PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Define library dependencies
# simulator_lib depends on elf_lib, decoder_lib, and registers_lib
target_link_libraries(simulator_lib
    PUBLIC
    elf_lib
    decoder_lib
    registers_lib
)

# The main executable depends on all libraries
target_link_libraries(SRRArch-simulator
    PRIVATE
    simulator_lib
    utils_lib
    ${LLVM_LIBRARIES}
)

# Install target (optional)
install(TARGETS SRRArch-simulator DESTINATION bin)