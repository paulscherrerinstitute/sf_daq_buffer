file(GLOB SOURCES
        src/*.cpp)

add_library(sf-buffer-lib STATIC ${SOURCES})
target_include_directories(sf-buffer-lib PUBLIC include/)
target_link_libraries(sf-buffer-lib
        external
        core-buffer-lib)

add_executable(sf-buffer src/main.cpp)
set_target_properties(sf-buffer PROPERTIES OUTPUT_NAME sf_buffer)
target_link_libraries(sf-buffer
        core-buffer-lib
        sf-buffer-lib
        zmq)

enable_testing()
add_subdirectory(test/)
