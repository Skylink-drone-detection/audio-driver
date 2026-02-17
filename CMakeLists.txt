cmake_minimum_required(VERSION 3.16)
project(audio_driver C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Katalogi
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Źródła - potentiometer (dawny mcp4011)
set(SRC_FILES
    src/adc.c
    src/potentiometer.c        # ← ZMIENIONE z mcp4011_driver.c
    src/shm_writer.c
    src/switch_logic.c
)

# bcm2835 (GitHub Actions zainstaluje)
find_library(BCM2835_LIB bcm2835)
find_path(BCM2835_INCLUDE bcm2835.h)

# Main
add_executable(main main.c ${SRC_FILES})
target_include_directories(main PRIVATE 
    ${INCLUDE_DIR} 
    ${BCM2835_INCLUDE}
)
target_link_libraries(main PRIVATE 
    ${BCM2835_LIB} 
    pthread m rt
)

# Testy (potentiometer_test.c zamiast mcp...)
enable_testing()
add_executable(potentiometer_test tests/potentiometer_test.c ${SRC_FILES})
target_include_directories(potentiometer_test PRIVATE ${INCLUDE_DIR} ${BCM2835_INCLUDE})
target_link_libraries(potentiometer_test PRIVATE ${BCM2835_LIB} pthread m rt)

add_executable(shm_writer_test tests/shm_writer_test.c ${SRC_FILES})
target_include_directories(shm_writer_test PRIVATE ${INCLUDE_DIR} ${BCM2835_INCLUDE})
target_link_libraries(shm_writer_test PRIVATE ${BCM2835_LIB} pthread m rt)
add_test(NAME SHMTest COMMAND shm_writer_test)

# DOXYGEN
find_package(Doxygen COMPONENTS dot)
if(DOXYGEN_FOUND)
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    # Konfiguracja Doxygen
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    message("Doxygen build started")

    # HTML + LaTeX
    add_custom_target(doc_doxygen
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )
endif()

