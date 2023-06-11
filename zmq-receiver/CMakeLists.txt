file(GLOB SOURCES src/*.cpp)

add_library(zmq-receiver-lib STATIC ${SOURCES})
target_include_directories(zmq-receiver-lib PUBLIC include/)
target_link_libraries(zmq-receiver-lib external core-buffer-lib)

add_executable(zmq-receiver src/main.cpp)
set_target_properties(zmq-receiver PROPERTIES OUTPUT_NAME zmq_receiver)
target_link_libraries(zmq-receiver zmq-receiver-lib zmq rt pthread hdf5 hdf5_cpp)

enable_testing()
add_subdirectory(test/)
