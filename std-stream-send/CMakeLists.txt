file(GLOB SOURCES
        src/*.cpp)

add_library(std-stream-send-lib STATIC ${SOURCES})
target_include_directories(std-stream-send-lib PUBLIC include/)
target_link_libraries(std-stream-send-lib
        external
        core-buffer-lib)

add_executable(std-stream-send src/main.cpp)
set_target_properties(std-stream-send PROPERTIES OUTPUT_NAME std_stream_send)
target_link_libraries(std-stream-send
        external
        core-buffer-lib
        std-stream-send-lib
        zmq
        pthread
        rt)
 
enable_testing()
add_subdirectory(test/)
