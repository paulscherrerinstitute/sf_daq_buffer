add_executable(core-buffer_tests main.cpp)

target_link_libraries(core-buffer_tests
        external
        core-buffer
        hdf5
        hdf5_hl
        hdf5_cpp
        zmq
        gtest)

add_executable(perf-sf_writer perf/perf_SFWriter.cpp)
target_link_libraries(perf-sf_writer
        core-buffer
        hdf5
        hdf5_hl
        hdf5_cpp
        gtest)

add_executable(perf-compression perf/perf_compression.cpp)
target_link_libraries(perf-compression
        external)