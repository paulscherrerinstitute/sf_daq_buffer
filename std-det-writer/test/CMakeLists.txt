add_executable(std-det-writer-tests main.cpp)

target_link_libraries(std-det-writer-tests
        std-det-writer-lib
        zmq
        rt
        gtest
        )
