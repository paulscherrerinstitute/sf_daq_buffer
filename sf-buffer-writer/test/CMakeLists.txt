add_executable(jf-live-daq-tests main.cpp)

target_link_libraries(jf-live-daq-tests
        jf-live-daq-lib
        hdf5
        hdf5_hl
        hdf5_cpp
        zmq
        gtest
        )
