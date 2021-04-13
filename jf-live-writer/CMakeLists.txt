find_package(MPI REQUIRED)
# Because of openmpi.
add_definitions(-DOMPI_SKIP_MPICXX)

file(GLOB SOURCES
        src/*.cpp)

add_library(jf-live-writer-lib STATIC ${SOURCES})
target_include_directories(jf-live-writer-lib
        PUBLIC include/
        SYSTEM ${MPI_INCLUDE_PATH})

target_link_libraries(jf-live-writer-lib
        external
        core-buffer-lib
        ${MPI_LIBRARIES}
        )

add_executable(jf-live-writer src/main.cpp)


if (USE_EIGER)
        set (LIB_NAME_UDP_RECV "eiger_live_writer")
else()
        set (LIB_NAME_UDP_RECV "jf_live_writer")
endif()

set_target_properties(jf-live-writer PROPERTIES OUTPUT_NAME ${LIB_NAME_UDP_RECV})

target_link_libraries(jf-live-writer
        jf-live-writer-lib
        zmq
        hdf5
        rt
        )

enable_testing()
add_subdirectory(test/)