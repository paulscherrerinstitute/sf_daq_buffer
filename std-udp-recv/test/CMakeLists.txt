add_executable(std-udp-recv-tests main.cpp)

target_link_libraries(std-udp-recv-tests
        core-buffer-lib
        std-udp-recv-lib
        gtest
        )

