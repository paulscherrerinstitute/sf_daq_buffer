file(GLOB SOURCES
        src/*.cpp)

add_library(jf-live-writer-lib STATIC ${SOURCES})
target_include_directories(jf-live-writer-lib PUBLIC include/)
target_link_libraries(jf-live-writer-lib
        external
        core-buffer-lib)

add_executable(jf-live-writer src/main.cpp)
set_target_properties(jf-live-writer PROPERTIES OUTPUT_NAME jf_live_writer)
target_link_libraries(jf-live-writer
        jf-live-writer-lib
        zmq
        rt
        )

enable_testing()
add_subdirectory(test/)