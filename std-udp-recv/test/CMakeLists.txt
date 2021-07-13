add_executable(std-udp-recv-tests main.cpp)


target_link_libraries(std-udp-recv-tests
        core-buffer-lib
        std-udp-recv-lib
        gtest
        )

add_executable(std-udp-sim simulator.cpp)
set_target_properties(std-udp-sim PROPERTIES OUTPUT_NAME std_udp_sim)
target_link_libraries(std-udp-sim
        core-buffer-lib
        )
