namespace sync_config
{
    // If the modules are offset more than 1000 pulses, crush.
    const uint64_t PULSE_OFFSET_LIMIT = 100;

    // Number of times we try to re-sync in case of failure.
    const int SYNC_RETRY_LIMIT = 3;

    // Number of pulses between each statistics print out.
    const size_t SYNC_STATS_MODULO = 1000;
}
