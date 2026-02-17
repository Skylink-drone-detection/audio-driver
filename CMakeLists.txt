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
    #src/switch_logic.c - not working
)

# Main executable
add_executable(main main.c ${SRC_FILES})
target_include_directories(main PRIVATE ${INCLUDE_DIR})

# Tests (Using CTest)
enable_testing()

# Register tests and create executables
add_executable(shm_writer_test tests/shm_writer_test.c ${SRC_FILES})
target_include_directories(shm_writer_test PRIVATE ${INCLUDE_DIR})
set_target_properties(shm_writer_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests)
add_test(NAME SharedMemoryWriterTest COMMAND shm_writer_test)

# --- Template for adding more tests ---
# add_executable(test_something tests/test_something.c ${SRC_FILES})
# target_include_directories(test_something PRIVATE ${INCLUDE_DIR})
# set_target_properties(test_something PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests)
# add_test(NAME SomethingTest COMMAND test_something)
