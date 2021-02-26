file(GLOB SOURCES
        src/*.cpp)

add_library(jf-buffer-writer-lib STATIC ${SOURCES})
target_include_directories(jf-buffer-writer-lib PUBLIC include/)
target_link_libraries(jf-buffer-writer-lib
        external
        core-buffer-lib)

add_executable(jf-buffer-writer src/main.cpp)
set_target_properties(jf-buffer-writer PROPERTIES OUTPUT_NAME jf_buffer_writer)
target_link_libraries(jf-buffer-writer
        jf-buffer-writer-lib
        zmq
        rt)

enable_testing()
add_subdirectory(test/)
