file(GLOB SOURCES src/*.cpp)

# add_library(sim-jungfrau-lib STATIC ${SOURCES})
# target_include_directories(sim-jungfrau-lib PUBLIC include/)
# target_link_libraries(sim-jungfrau-lib external core-buffer-lib)

add_executable(sim-jungfrau src/main.cpp)
set_target_properties(sim-jungfrau PROPERTIES OUTPUT_NAME sim_jungfrau)
target_link_libraries(sim-jungfrau zmq rt pthread)

enable_testing()
add_subdirectory(test/)
