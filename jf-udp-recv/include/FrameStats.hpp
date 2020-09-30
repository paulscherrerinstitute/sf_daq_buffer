#include <cstddef>
#include <formats.hpp>
#include <chrono>

#ifndef SF_DAQ_BUFFER_FRAMESTATS_HPP
#define SF_DAQ_BUFFER_FRAMESTATS_HPP


class FrameStats {
    const std::string source_name_;
    size_t stats_modulo_;

    int frames_counter_;
    int n_missed_packets_;
    int n_corrupted_frames_;
    std::chrono::time_point<std::chrono::steady_clock> stats_interval_start_;

    void reset_counters();
    void print_stats();

public:
    FrameStats(const std::string &source_name, const size_t stats_modulo);
    void record_stats(const ModuleFrame &meta);
};


#endif //SF_DAQ_BUFFER_FRAMESTATS_HPP
