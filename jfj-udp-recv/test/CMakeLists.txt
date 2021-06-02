add_executable(jfj-udp-recv-tests main.cpp)

target_link_libraries(jfj-udp-recv-tests
        core-buffer-lib
        jfj-udp-recv-lib
        gtest
        )

