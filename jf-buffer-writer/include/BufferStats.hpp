#include <cstddef>
#include <formats.hpp>
#include <chrono>

#ifndef SF_DAQ_BUFFER_FRAMESTATS_HPP
#define SF_DAQ_BUFFER_FRAMESTATS_HPP


class BufferStats {
    const std::string detector_name_;
    const int module_id_;
    size_t stats_modulo_;

    int frames_counter_;
    uint32_t total_buffer_write_us_;
    uint32_t max_buffer_write_us_;
    std::chrono::time_point<std::chrono::steady_clock> stats_interval_start_;

    void reset_counters();
    void print_stats();

public:
    BufferStats(
            const std::string &detector_name,
            const int module_id,
            const size_t stats_modulo);
    void start_frame_write();
    void end_frame_write();
};


#endif //SF_DAQ_BUFFER_FRAMESTATS_HPP
