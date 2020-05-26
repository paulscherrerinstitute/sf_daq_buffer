file(GLOB SOURCES
        src/*.cpp)

add_library(sf-replay-lib STATIC ${SOURCES})
target_include_directories(sf-replay-lib PUBLIC include/)
target_link_libraries(sf-replay-lib
        external
        core-buffer-lib)

add_executable(sf-replay src/main.cpp)
set_target_properties(sf-replay PROPERTIES OUTPUT_NAME sf_replay)
target_link_libraries(sf-replay
        core-buffer-lib
        sf-replay-lib
        zmq
        hdf5
        hdf5_cpp
        pthread)

enable_testing()
add_subdirectory(test/)