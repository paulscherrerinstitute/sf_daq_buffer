#include <cstddef>

namespace live_writer_config
{
    // MS to retry reading from the image assembler.
    const size_t ASSEMBLER_RETRY_MS = 5;
    // Number of slots in the reconstruction buffer.
    const size_t WRITER_IA_N_SLOTS = 200;
}