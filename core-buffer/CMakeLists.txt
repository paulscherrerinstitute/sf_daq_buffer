add_subdirectory(external/)

file(GLOB SOURCES
        src/*.cpp)

add_library(core-buffer STATIC ${SOURCES})
target_include_directories(core-buffer PUBLIC include/)
target_include_directories(core-buffer PUBLIC external/)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(core-buffer PRIVATE DEBUG_OUTPUT)
endif()

enable_testing()
add_subdirectory(test/)