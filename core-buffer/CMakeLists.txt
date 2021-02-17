add_subdirectory(external/)

file(GLOB SOURCES
        src/*.cpp)

add_library(core-buffer-lib STATIC ${SOURCES})
target_include_directories(core-buffer-lib PUBLIC include/)
target_link_libraries(core-buffer-lib
        external)

if (USE_EIGER)
        target_compile_definitions(core-buffer-lib PUBLIC -DUSE_EIGER)
endif (USE_EIGER)

if (DEBUG_OUTPUT)
        target_compile_definitions(core-buffer-lib PUBLIC -DDEBUG_OUTPUT)
endif(DEBUG_OUTPUT)

enable_testing()
add_subdirectory(test/)