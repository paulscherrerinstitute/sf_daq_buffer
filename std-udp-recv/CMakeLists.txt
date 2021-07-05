file(GLOB SOURCES
        src/*.cpp)

add_library(std-udp-recv-lib STATIC ${SOURCES})
target_include_directories(std-udp-recv-lib PUBLIC include/)
target_link_libraries(std-udp-recv-lib
        external
        core-buffer-lib)

add_executable(std-udp-recv src/main.cpp)

set_target_properties(std-udp-recv PROPERTIES OUTPUT_NAME std_udp_recv)

target_link_libraries(std-udp-recv
        std-udp-recv-lib
        zmq
        rt)

enable_testing()
add_subdirectory(test/)
