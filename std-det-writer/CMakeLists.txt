find_package(MPI REQUIRED)
# Because of openmpi.
add_definitions(-DOMPI_SKIP_MPICXX)

file(GLOB SOURCES
        src/*.cpp)

add_library(std-det-writer-lib STATIC ${SOURCES})
target_include_directories(std-det-writer-lib
        PUBLIC include/
        SYSTEM ${MPI_INCLUDE_PATH})

target_link_libraries(std-det-writer-lib
        external
        core-buffer-lib
        ${MPI_LIBRARIES}
        )

add_executable(std-det-writer src/main.cpp)

set_target_properties(std-det-writer PROPERTIES OUTPUT_NAME std_det_writer)

target_link_libraries(std-det-writer
        std-det-writer-lib
        zmq
        hdf5
        rt
        )

enable_testing()
add_subdirectory(test/)