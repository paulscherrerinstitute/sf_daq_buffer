namespace assembler_config
{
    // N of IO threads to send image metadata.
    const int ASSEMBLER_ZMQ_IO_THREADS = 1;

    // If the modules are offset more than 1000 pulses, crush.
    const uint64_t PULSE_OFFSET_LIMIT = 100;

    // Number of times we try to re-sync in case of failure.
    const int SYNC_RETRY_LIMIT = 3;

    // Number of pulses between each statistics print out.
    const size_t ASSEMBLER_STATS_MODULO = 1000;
}
