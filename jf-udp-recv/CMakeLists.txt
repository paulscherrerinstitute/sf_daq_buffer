file(GLOB SOURCES
        src/*.cpp)

add_library(sf-buffer-recv-lib STATIC ${SOURCES})
target_include_directories(sf-buffer-recv-lib PUBLIC include/)
target_link_libraries(sf-buffer-recv-lib
        external
        core-buffer-lib)

add_executable(sf-buffer-recv src/main.cpp)
set_target_properties(sf-buffer-recv PROPERTIES OUTPUT_NAME sf_buffer)
target_link_libraries(sf-buffer-recv
        sf-buffer-recv-lib
        zmq
        rt)

enable_testing()
add_subdirectory(test/)
