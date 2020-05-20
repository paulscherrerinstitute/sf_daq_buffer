add_executable(sf-buffer-tests main.cpp)

target_link_libraries(sf-buffer-tests
        core-buffer-lib
        sf-buffer-lib
        external
        hdf5
        hdf5_hl
        hdf5_cpp
        zmq
        gtest)

