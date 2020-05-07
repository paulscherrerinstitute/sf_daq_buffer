add_executable(sf-buffer src/sf_buffer.cpp)
set_target_properties(sf-buffer PROPERTIES OUTPUT_NAME sf_buffer)
target_link_libraries(sf-buffer
        core-buffer
        zmq
        hdf5
        hdf5_hl
        hdf5_cpp
        boost_system
        pthread)

#add_executable(sf-reader src/sf_reader.cpp)
#set_target_properties(sf-reader PROPERTIES OUTPUT_NAME sf_reader)
#target_link_libraries(sf-reader
#        core-buffer
#        zmq
#        hdf5
#        hdf5_hl
#        hdf5_cpp
#        boost_system
#        pthread)

add_executable(sf-replay src/sf_replay.cpp)
set_target_properties(sf-replay PROPERTIES OUTPUT_NAME sf_replay)
target_link_libraries(sf-replay
        core-buffer
        zmq
        hdf5
        hdf5_hl
        hdf5_cpp
        boost_system
        pthread)

#add_executable(sf-live src/sf_live.cpp)
#set_target_properties(sf-live PROPERTIES OUTPUT_NAME sf_live)
#target_link_libraries(sf-live
#        core-buffer
#        zmq
#        hdf5
#        hdf5_hl
#        hdf5_cpp
#        boost_system
#        pthread)

add_executable(sf-writer src/sf_writer.cpp)
set_target_properties(sf-writer PROPERTIES OUTPUT_NAME sf_writer)
target_link_libraries(sf-writer
        core-buffer
        zmq
        hdf5
        hdf5_hl
        hdf5_cpp
        boost_system
        pthread)

add_executable(sf-stream src/sf_stream.cpp)
set_target_properties(sf-stream PROPERTIES OUTPUT_NAME sf_stream)
target_link_libraries(sf-stream
        core-buffer
        zmq
        jsoncpp
        boost_system
        pthread) 

enable_testing()
add_subdirectory(test/)
