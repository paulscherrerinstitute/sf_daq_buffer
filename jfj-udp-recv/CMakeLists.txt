file(GLOB SOURCES src/*.cpp)

add_library(jfj-udp-recv-lib STATIC ${SOURCES})
target_include_directories(jfj-udp-recv-lib PUBLIC include/)
target_link_libraries(jfj-udp-recv-lib external core-buffer-lib)

add_executable(jfj-udp-recv src/main.cpp)
set_target_properties(jfj-udp-recv PROPERTIES OUTPUT_NAME jf_udp_recv)
target_link_libraries(jfj-udp-recv jfj-udp-recv-lib zmq rt)

enable_testing()
add_subdirectory(test/)
