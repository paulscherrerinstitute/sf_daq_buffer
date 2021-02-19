file(GLOB SOURCES
        src/*.cpp)

add_library(sf-stream-lib STATIC ${SOURCES})
target_include_directories(sf-stream-lib PUBLIC include/)
target_link_libraries(sf-stream-lib
        external
        core-buffer-lib)

add_executable(sf-stream src/main.cpp)

if (USE_EIGER)
        set (LIB_NAME_STREAMER "eiger_stream")
else()
        set (LIB_NAME_STREAMER "sf_stream")
endif()

set_target_properties(sf-stream PROPERTIES OUTPUT_NAME ${LIB_NAME_STREAMER})
target_link_libraries(sf-stream
        external
        core-buffer-lib
        sf-stream-lib
        zmq
        pthread
        rt)

enable_testing()
add_subdirectory(test/)
