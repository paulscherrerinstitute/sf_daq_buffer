add_executable(jf-assembler-tests main.cpp)

target_link_libraries(jf-assembler-tests
        jf-assembler-lib
        gtest
        )

