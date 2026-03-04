cmake_minimum_required(VERSION 3.16)
project(audio_driver C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O2")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -O0")

set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(SRC_FILES
    src/adc.c
    src/potentiometer.c
    src/shm_writer.c
    src/switch_logic.c
)

find_library(BCM2835_LIB bcm2835)
find_path(BCM2835_INCLUDE bcm2835.h)

add_executable(main main.c ${SRC_FILES})
target_include_directories(main PRIVATE 
    ${INCLUDE_DIR} 
    ${BCM2835_INCLUDE}
)
target_link_libraries(main PRIVATE 
    ${BCM2835_LIB} 
    pthread m rt
)

enable_testing()
add_executable(potentiometer_test tests/potentiometer_test.c ${SRC_FILES})
target_include_directories(potentiometer_test PRIVATE ${INCLUDE_DIR} ${BCM2835_INCLUDE})
target_link_libraries(potentiometer_test PRIVATE ${BCM2835_LIB} pthread m rt)

add_executable(shm_writer_test tests/shm_writer_test.c ${SRC_FILES})
target_include_directories(shm_writer_test PRIVATE ${INCLUDE_DIR} ${BCM2835_INCLUDE})
target_link_libraries(shm_writer_test PRIVATE ${BCM2835_LIB} pthread m rt)
add_test(NAME SHMTest COMMAND shm_writer_test)

find_package(Doxygen COMPONENTS dot)
if(DOXYGEN_FOUND)
    set(DOXYFILE_IN  ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    set(DOXYFILE_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)

    add_custom_target(doc
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating documentation (HTML + LaTeX)..."
        VERBATIM
    )
    message(STATUS "Doxygen: make doc")
else()
    message(WARNING "Doxygen NOT FOUND. Install: sudo apt install doxygen graphviz")
endif()
