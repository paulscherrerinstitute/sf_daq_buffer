add_executable(core-buffer_tests main.cpp)

target_link_libraries(core-buffer_tests
        core-buffer
        external
        hdf5
        hdf5_hl
        hdf5_cpp
        zmq
        gtest)

add_executable(perf-sf_writer perf/perf_WriterH5Writer.cpp)
target_link_libraries(perf-sf_writer
        core-buffer
        hdf5
        hdf5_hl
        hdf5_cpp
        gtest)
