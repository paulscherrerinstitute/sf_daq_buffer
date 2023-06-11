add_executable(jf-zmqstreamer-tests main.cpp)

target_link_libraries(jf-zmqstreamer-tests
        core-buffer-lib
        jf-zmqstreamer-lib
        gtest
        )

