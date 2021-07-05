file(GLOB SOURCES
        src/*.cpp)

add_library(std-udp-sync-lib STATIC ${SOURCES})
target_include_directories(std-udp-sync-lib PUBLIC include/)
target_link_libraries(std-udp-sync-lib
        external
        core-buffer-lib)

add_executable(std-udp-sync src/main.cpp)

set_target_properties(std-udp-sync PROPERTIES OUTPUT_NAME std_udp_sync)
target_link_libraries(std-udp-sync
        external
        core-buffer-lib
        std-udp-sync-lib
        zmq
        pthread
        rt)

enable_testing()
add_subdirectory(test/)
