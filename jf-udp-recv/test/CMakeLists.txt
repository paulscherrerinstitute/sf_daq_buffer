add_executable(jf-udp-recv-tests main.cpp)

target_link_libraries(jf-udp-recv-tests
        core-buffer-lib
        jf-udp-recv-lib
        gtest
        )

