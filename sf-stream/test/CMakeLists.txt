add_executable(sf-stream-tests main.cpp)

target_link_libraries(sf-stream-tests
        sf-stream-lib
        gtest
        )

