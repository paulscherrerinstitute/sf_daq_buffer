file(GLOB SOURCES
        src/*.cpp)

add_library(sf-writer-lib STATIC ${SOURCES})
target_include_directories(sf-writer-lib PUBLIC include/)
target_link_libraries(sf-writer-lib
        external
        core-buffer-lib)

add_executable(sf-writer src/main.cpp)
set_target_properties(sf-writer PROPERTIES OUTPUT_NAME sf_writer)
target_link_libraries(sf-writer
        sf-writer-lib
        zmq
        hdf5
        hdf5_hl
        hdf5_cpp
        pthread
        )

enable_testing()
add_subdirectory(test/)