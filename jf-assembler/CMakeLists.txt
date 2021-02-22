file(GLOB SOURCES
        src/*.cpp)

add_library(jf-assembler-lib STATIC ${SOURCES})
target_include_directories(jf-assembler-lib PUBLIC include/)
target_link_libraries(jf-assembler-lib
        external
        core-buffer-lib)

add_executable(jf-assembler src/main.cpp)

if (USE_EIGER)
        set (LIB_NAME_ASSEMBLER "eiger_assembler")
else()
        set (LIB_NAME_ASSEMBLER "jf_assembler")
endif()

set_target_properties(jf-assembler PROPERTIES OUTPUT_NAME ${LIB_NAME_ASSEMBLER})
target_link_libraries(jf-assembler
        external
        core-buffer-lib
        jf-assembler-lib
        zmq
        pthread
        rt)

enable_testing()
add_subdirectory(test/)
