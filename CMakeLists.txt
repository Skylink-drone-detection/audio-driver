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

# libbcm2835 - BUILD Z SOURCE (zero zależności!)
include(ExternalProject)
ExternalProject_Add(
    bcm2835
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/bcm2835-source
    CONFIGURE_COMMAND ./configure --prefix=<INSTALL_DIR> --enable-static
    BUILD_COMMAND make CFLAGS="-fPIC"
    INSTALL_COMMAND make install
    BUILD_BYPRODUCTS <INSTALL_DIR>/lib/libbcm2835.a
)

ExternalProject_Get_Property(bcm2835 INSTALL_DIR BINARY_DIR)

# Main executable
add_executable(main main.c ${SRC_FILES})
target_include_directories(main PRIVATE 
    ${INCLUDE_DIR}
    ${BINARY_DIR}
    $<TARGET_PROPERTY:bcm2835,INSTALL_DIR>/include
)

add_dependencies(main bcm2835)
target_link_libraries(main PRIVATE 
    ${BINARY_DIR}/libbcm2835.a
    pthread 
    m
)

# Tests
enable_testing()
add_executable(shm_writer_test tests/shm_writer_test.c ${SRC_FILES})
target_include_directories(shm_writer_test PRIVATE ${INCLUDE_DIR} ${BINARY_DIR})
add_dependencies(shm_writer_test bcm2835)
target_link_libraries(shm_writer_test PRIVATE ${BINARY_DIR}/libbcm2835.a pthread)
set_target_properties(shm_writer_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests)
add_test(NAME SharedMemoryWriterTest COMMAND shm_writer_test)

message(STATUS "=== ZERO DEPENDENCIES BUILD ===")
message(STATUS "libbcm2835: BUILT FROM SOURCE ✅")
