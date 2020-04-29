add_executable(sf-buffer_tests test_main.cpp)

target_link_libraries(sf-buffer_tests
        core-buffer
        gtest
        zmq
        hdf5
        hdf5_hl
        hdf5_cpp)