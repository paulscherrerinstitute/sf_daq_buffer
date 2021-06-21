file(GLOB SOURCES src/*.cpp)

add_library(jfj-combined-lib STATIC ${SOURCES})
target_include_directories(jfj-combined-lib PUBLIC include/)
target_link_libraries(jfj-combined-lib external core-buffer-lib)

add_executable(jfj-combined src/main.cpp)
set_target_properties(jfj-combined PROPERTIES OUTPUT_NAME jf_udp_recv)
target_link_libraries(jfj-combined jfj-combined-lib zmq rt)

enable_testing()
add_subdirectory(test/)