file(GLOB SOURCES
        src/*.cpp)

add_library(jf-udp-recv-lib STATIC ${SOURCES})
target_include_directories(jf-udp-recv-lib PUBLIC include/)
target_link_libraries(jf-udp-recv-lib
        external
        core-buffer-lib)

add_executable(jf-udp-recv src/main.cpp)
set_target_properties(jf-udp-recv PROPERTIES OUTPUT_NAME jf_udp_recv)
target_link_libraries(jf-udp-recv
        jf-udp-recv-lib
        zmq
        rt)

enable_testing()
add_subdirectory(test/)
