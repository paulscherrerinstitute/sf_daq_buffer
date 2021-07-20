#ifndef SF_DAQ_BUFFER_ASSEMBLERSTATS_HPP
#define SF_DAQ_BUFFER_ASSEMBLERSTATS_HPP

#include <chrono>
#include <string>
#include <formats.hpp>

class AssemblerStats {
    const std::string detector_name_;
    const size_t stats_time_;

    int image_counter_;
    int n_corrupted_images_;
    int n_sync_lost_images_;
    std::chrono::time_point<std::chrono::steady_clock> stats_interval_start_;

    void reset_counters();
    void print_stats();

public:
    AssemblerStats(const std::string &detector_name,
                   const size_t stats_time);

    void record_stats(const ImageMetadata *meta, const uint32_t n_lost_pulses);
};


#endif //SF_DAQ_BUFFER_ASSEMBLERSTATS_HPP
