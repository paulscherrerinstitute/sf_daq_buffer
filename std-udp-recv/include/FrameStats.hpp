#include <cstddef>
#include <formats.hpp>
#include <chrono>

#ifndef SF_DAQ_BUFFER_FRAMESTATS_HPP
#define SF_DAQ_BUFFER_FRAMESTATS_HPP


class FrameStats {
    const std::string detector_name_;
    const int n_modules_;
    const int module_id_;
    const int bit_depth_;
    const int n_packets_per_frame_;
    size_t stats_time_;

    int frames_counter_;
    int n_missed_packets_;
    int n_corrupted_frames_;
    int n_corrupted_pulse_id_;
    std::chrono::time_point<std::chrono::steady_clock> stats_interval_start_;

    void reset_counters();
    void print_stats();

public:////config.detector_name, n_receivers, module_id, bit_depth, STATS_TIME
    FrameStats(const std::string &detector_name,
               const int n_modules,
               const int module_id,
               const int bit_depth,
               const size_t stats_time);
    void record_stats(const ModuleFrame &meta, const bool bad_pulse_id);
};


#endif //SF_DAQ_BUFFER_FRAMESTATS_HPP
