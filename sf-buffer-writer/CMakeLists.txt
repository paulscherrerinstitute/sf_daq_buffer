
find_package(MPI REQUIRED)
# Because of openmpi.
add_definitions(-DOMPI_SKIP_MPICXX)

file(GLOB SOURCES
        src/*.cpp)

add_library(jf-live-daq-lib STATIC ${SOURCES})
target_include_directories(jf-live-daq-lib
        PUBLIC include/
        SYSTEM ${MPI_INCLUDE_PATH})

target_link_libraries(jf-live-daq-lib
        external
        core-buffer-lib
        ${MPI_LIBRARIES})

add_executable(jf-live-daq src/main.cpp)
set_target_properties(jf-live-daq PROPERTIES OUTPUT_NAME jf_live_daq)
target_link_libraries(jf-live-daq
        jf-live-daq-lib
        )

enable_testing()
add_subdirectory(test/)