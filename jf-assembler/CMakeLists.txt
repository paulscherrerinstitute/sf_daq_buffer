file(GLOB SOURCES
        src/*.cpp)

add_library(jf-assembler-lib STATIC ${SOURCES})
target_include_directories(jf-assembler-lib PUBLIC include/)
target_link_libraries(jf-assembler-lib
        external
        core-buffer-lib)

add_executable(jf-assembler src/main.cpp)
set_target_properties(jf-assembler PROPERTIES OUTPUT_NAME jf_assembler)
target_link_libraries(jf-assembler
        external
        core-buffer-lib
        jf-assembler-lib
        zmq
        pthread
        rt)

enable_testing()
add_subdirectory(test/)
