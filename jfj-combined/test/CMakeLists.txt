add_executable(jfj-combined-tests main.cpp)

target_link_libraries(jfj-combined-tests
        core-buffer-lib
        jfj-combined-lib
        gtest
        )

