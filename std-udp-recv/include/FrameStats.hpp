#include <cstddef>
#include <formats.hpp>
#include <chrono>

#ifndef SF_DAQ_BUFFER_FRAMESTATS_HPP
#define SF_DAQ_BUFFER_FRAMESTATS_HPP


class FrameStats {
    const std::string detector_name_;
    const int module_id_;
    const int n_packets_per_frame_;
    const size_t stats_time_;

    int frames_counter_;
    int n_missed_packets_;
    int n_corrupted_frames_;
    std::chrono::time_point<std::chrono::steady_clock> stats_interval_start_;

    void reset_counters();
    void print_stats();

public:
    FrameStats(std::string detector_name,
               const int module_id,
               const int n_packets_per_frame,
               const size_t stats_time);
    void record_stats(const ModuleFrame &meta);
};


#endif //SF_DAQ_BUFFER_FRAMESTATS_HPP
