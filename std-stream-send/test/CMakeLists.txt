add_executable(std-stream-send-tests main.cpp)

target_link_libraries(std-stream-send-tests
        std-stream-send-lib
        gtest
        )

