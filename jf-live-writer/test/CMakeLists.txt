add_executable(jf-live-writer-tests main.cpp)

target_link_libraries(jf-live-writer-tests
        jf-live-writer-lib
        zmq
        rt
        gtest
        )
