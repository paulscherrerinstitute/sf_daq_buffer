cmake_minimum_required(VERSION 3.12)

project(lib_cpp_h5_writer)
set(CMAKE_CXX_STANDARD 11)

set (LIB_CPP_H5_WRITER_VERSION "1.0.0")

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Download and unpack googletest at configure time

configure_file(googletest.in googletest-download/CMakeLists.txt)

execute_process(
        COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" .
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/googletest-download"
        )

execute_process(
        COMMAND "${CMAKE_COMMAND}" --build .
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/googletest-download"
        )

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

add_subdirectory(
        ${CMAKE_CURRENT_BINARY_DIR}/googletest-src
        ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
        EXCLUDE_FROM_ALL)

add_subdirectory("core-writer")
add_subdirectory("csaxs")
add_subdirectory("sf")