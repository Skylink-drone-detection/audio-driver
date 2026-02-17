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

# Sprawdź czy bcm2835 source istnieje
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/bcm2835-source")
    message(FATAL_ERROR 
        "bcm2835 source NOT FOUND!\n"
        "1. wget https://github.com/matthiasbock/bcm2835/archive/refs/heads/master.zip\n"
        "2. unzip master.zip\n"
        "3. mv bcm2835-master external/bcm2835-source\n"
        "4. git add external/bcm2835-source && git commit -m 'Add bcm2835'"
    )
endif()

# libbcm2835 - BUILD Z SOURCE
include(ExternalProject)
ExternalProject_Add(
    bcm2835
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/bcm2835-source
    BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/external/bcm2835-build
    INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/external/bcm2835-install
    CONFIGURE_COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/external/bcm2835-source/configure 
                      --prefix=<INSTALL_DIR>
                      CFLAGS="-fPIC"
    BUILD_COMMAND make -j4
    INSTALL_COMMAND make install
    BUILD_BYPRODUCTS <INSTALL_DIR>/lib/libbcm2835.a
)

# Pobierz ścieżki
ExternalProject_Get_Property(bcm2835 INSTALL_DIR BINARY_DIR)

# Main executable
add_executable(main main.c ${SRC_FILES})
add_dependencies(main bcm2835)

target_include_directories(main PRIVATE 
    ${INCLUDE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/external/bcm2835-install/include
    ${BINARY_DIR}
)

target_link_libraries(main PRIVATE 
    ${CMAKE_CURRENT_BINARY_DIR}/external/bcm2835-install/lib/libbcm2835.a
    pthread 
    m
)

# Tests
enable_testing()
add_executable(shm_writer_test tests/shm_writer_test.c ${SRC_FILES})
add_dependencies(shm_writer_test bcm2835)
target_include_directories(shm_writer_test PRIVATE 
    ${INCLUDE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/external/bcm2835-install/include
    ${BINARY_DIR}
)
target_link_libraries(shm_writer_test PRIVATE 
    ${CMAKE_CURRENT_BINARY_DIR}/external/bcm2835-install/lib/libbcm2835.a
    pthread
)
set_target_propertie
