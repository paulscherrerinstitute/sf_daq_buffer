add_executable(sf-buffer-writer-tests main.cpp)

target_link_libraries(sf-buffer-writer-tests
        sf-buffer-writer-lib
        zmq
        gtest
        )
