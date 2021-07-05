add_executable(std-udp-sync-tests main.cpp)

target_link_libraries(std-udp-sync-tests
        std-udp-sync-lib
        gtest
        )

