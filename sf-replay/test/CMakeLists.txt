add_executable(sf-replay-tests main.cpp)

target_link_libraries(sf-replay-tests
        core-buffer-lib
        sf-buffer-lib
        sf-replay-lib
        hdf5
        hdf5_cpp
        gtest
        )

