add_executable(sf-writer-tests main.cpp)

target_link_libraries(sf-writer-tests
        sf-writer-lib
        hdf5
        hdf5_hl
        hdf5_cpp
        zmq
        gtest
        )
