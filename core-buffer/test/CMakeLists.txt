add_executable(core-buffer-tests main.cpp)

target_link_libraries(core-buffer-tests
        core-buffer-lib
        external
        rt
        hdf5
        hdf5_cpp
        zmq
        gtest)
