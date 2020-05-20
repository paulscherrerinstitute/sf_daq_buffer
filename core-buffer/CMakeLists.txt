add_subdirectory(external/)

file(GLOB SOURCES
        src/*.cpp)

add_library(core-buffer-lib STATIC ${SOURCES})
target_include_directories(core-buffer-lib PUBLIC include/)

#if(CMAKE_BUILD_TYPE STREQUAL "Debug")
#    target_compile_definitions(core-buffer-lib PRIVATE DEBUG_OUTPUT)
#endif()

#add_executable(sf-replay src/replay/sf_replay.cpp)
#set_target_properties(sf-replay PROPERTIES OUTPUT_NAME sf_replay)
#target_link_libraries(sf-replay
#        core-buffer
#        external
#        zmq
#        hdf5
#        hdf5_hl
#        hdf5_cpp
#        boost_system
#        pthread)
#
#add_executable(sf-writer src/writer/sf_writer.cpp)
#set_target_properties(sf-writer PROPERTIES OUTPUT_NAME sf_writer)
#target_link_libraries(sf-writer
#        core-buffer
#        external
#        zmq
#        hdf5
#        hdf5_hl
#        hdf5_cpp
#        boost_system
#        pthread)
#
#add_executable(sf-stream src/stream/sf_stream.cpp)
#set_target_properties(sf-stream PROPERTIES OUTPUT_NAME sf_stream)
#target_link_libraries(sf-stream
#        core-buffer
#        zmq
#        jsoncpp
#        boost_system
#        pthread)

enable_testing()
add_subdirectory(test/)