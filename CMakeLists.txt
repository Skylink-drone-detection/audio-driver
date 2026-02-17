cmake_minimum_required(VERSION 3.16)
project(audio_driver C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Include directories
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Source files
set(SRC_FILES
    src/mcp4011_driver.c
    src/shm_writer.c
)

# Sprawdź bcm2835 source
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/bcm2835-source")
    message(FATAL_ERROR "bcm2835 source missing! Download: https://github.com/matthiasbock/bcm2835")
endif()

# libbcm2835 - BUILD Z SOURCE (jedna linia na komendę)
include(ExternalProject)
ExternalProject_Add(bcm2835
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/bcm2835-source
    BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/bcm2835-build
    INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/bcm2835-install
    CONFIGURE_COMMAND ./configure --prefix=<INSTALL_DIR>
    BUILD_COMMAND make -j4
    INSTALL_COMMAND make install
)

# Pobierz ścieżki
ExternalProject_Get_Property(bcm2835 INSTALL_DIR BINARY_DIR)

# Main executable
add_executable(main main.c ${SRC_FILES})
add_dependencies(main bcm2835)
target_include_directories(main PRIVATE 
    ${INCLUDE_DIR}
    ${BINARY_DIR}
    ${INSTALL_DIR}/include
)
target_link_libraries(main PRIVATE 
    ${INSTALL_DIR}/lib/libbcm2835.a
    pthread m
)

# Tests
enable_testing()
add_executable(shm_writer_test tests/shm_writer_test.c ${SRC_FILES})
add_dependencies(shm_writer_test bcm2835)
target_include_directories(shm_writer_test PRIVATE 
    ${INCLUDE_DIR}
    ${BINARY_DIR}
    ${INSTALL_DIR}/include
)
target_link_libraries(shm_writer_test PRIVATE 
    ${INSTALL_DIR}/lib/libbcm2835.a
    pthread
)
set_target_properties(shm_writer_test PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests
)
add_test(NAME SharedMemoryWriterTest COMMAND shm_writer_test)

message(STATUS "bcm2835: ZERO DEPENDENCIES - BUILT FROM SOURCE!")
