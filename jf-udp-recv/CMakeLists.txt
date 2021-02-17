file(GLOB SOURCES
        src/*.cpp)

add_library(jf-udp-recv-lib STATIC ${SOURCES})
target_include_directories(jf-udp-recv-lib PUBLIC include/)
target_link_libraries(jf-udp-recv-lib
        external
        core-buffer-lib)

add_executable(jf-udp-recv src/main.cpp)

if (USE_EIGER)
        set (LIB_NAME_UDP_RECV "eiger_udp_recv")
else()
        set (LIB_NAME_UDP_RECV "jf_udp_recv")
endif()

set_target_properties(jf-udp-recv PROPERTIES OUTPUT_NAME ${LIB_NAME_UDP_RECV})

target_link_libraries(jf-udp-recv
        jf-udp-recv-lib
        zmq
        rt)

enable_testing()
add_subdirectory(test/)
