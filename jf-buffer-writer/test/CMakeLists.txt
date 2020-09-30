add_executable(jf-buffer-writer-tests main.cpp)

target_link_libraries(jf-buffer-writer-tests
        jf-buffer-writer-lib
        zmq
        gtest
        )
