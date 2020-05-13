file(GLOB SOURCES
        bitshuffle/*.c
        bitshuffle/*.cpp)

add_library(external STATIC ${SOURCES})
target_include_directories(external PUBLIC .)
