add_executable(sf-buffer-recv-tests main.cpp)

target_link_libraries(sf-buffer-recv-tests
        core-buffer-lib
        sf-buffer-recv-lib
        gtest
        )

