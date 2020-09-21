file(GLOB SOURCES
        src/*.cpp)

add_library(sf-buffer-writer-lib STATIC ${SOURCES})
target_include_directories(sf-buffer-writer-lib PUBLIC include/)
target_link_libraries(sf-buffer-writer-lib
        external
        core-buffer-lib)

add_executable(sf-buffer-writer src/main.cpp)
set_target_properties(sf-buffer-writer PROPERTIES OUTPUT_NAME sf_buffer)
target_link_libraries(sf-buffer-writer
        sf-buffer-writer-lib
        zmq
        rt)

enable_testing()
add_subdirectory(test/)
