add_executable(sf-buffer-tests main.cpp)

target_link_libraries(sf-buffer-tests
        core-buffer-lib
        sf-buffer-lib
        hdf5
        hdf5_cpp
        gtest
        )

