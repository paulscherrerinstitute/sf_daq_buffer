add_subdirectory(external/)

file(GLOB SOURCES
        src/*.cpp)

add_library(core-buffer-lib STATIC ${SOURCES})
target_include_directories(core-buffer-lib PUBLIC include/)
target_link_libraries(core-buffer-lib
        external)

enable_testing()
add_subdirectory(test/)