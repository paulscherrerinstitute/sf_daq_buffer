file(GLOB SOURCES
        src/*.cpp)

add_library(sf-stream-lib STATIC ${SOURCES})
target_include_directories(sf-stream-lib PUBLIC include/)
target_link_libraries(sf-stream-lib
        external
        core-buffer-lib)

add_executable(sf-stream src/main.cpp)
set_target_properties(sf-stream PROPERTIES OUTPUT_NAME sf_stream)
target_link_libraries(sf-stream
        external
        sf-stream-lib
        zmq
        pthread
        jsoncpp)

enable_testing()
add_subdirectory(test/)
