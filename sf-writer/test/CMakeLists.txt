add_executable(sf-writer-tests main.cpp)

target_link_libraries(sf-writer-tests
        sf-writer-lib
        hdf5
        hdf5_cpp
        zmq
        gtest
        )

add_executable(sf-writer-recv manual/test_sf_writer_recv.cpp)
set_target_properties(sf-writer-recv PROPERTIES OUTPUT_NAME sf_writer)
set_target_properties(sf-writer-recv PROPERTIES EXCLUDE_FROM_ALL TRUE)
target_link_libraries(sf-writer-recv
        sf-writer-lib
        zmq
        hdf5
        hdf5_cpp
        pthread
        )