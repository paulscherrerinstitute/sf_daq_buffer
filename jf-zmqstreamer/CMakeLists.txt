file(GLOB SOURCES src/*.cpp)

add_library(jf-zmqstreamer-lib STATIC ${SOURCES})
target_include_directories(jf-zmqstreamer-lib PUBLIC include/)
target_link_libraries(jf-zmqstreamer-lib external core-buffer-lib)

add_executable(jf-zmqstreamer src/main.cpp)
set_target_properties(jf-zmqstreamer PROPERTIES OUTPUT_NAME jfj_combined)
target_link_libraries(jf-zmqstreamer jf-zmqstreamer-lib zmq rt pthread)

enable_testing()
add_subdirectory(test/)
