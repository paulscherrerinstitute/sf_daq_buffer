#ifndef SF_DAQ_BUFFER_STREAMSTATS_HPP
#define SF_DAQ_BUFFER_STREAMSTATS_HPP

#include "date.h"
#include <chrono>
#include <string>
#include <formats.hpp>

class StreamStats {
    const std::string detector_name_;
    const std::string stream_name_;
    const size_t stats_modulo_;

    int image_counter_;
    int n_corrupted_images_;
    std::chrono::time_point<std::chrono::steady_clock> stats_interval_start_;

    void reset_counters();
    void print_stats();

public:
    StreamStats(const std::string &detector_name,
                const std::string &stream_name,
                const size_t stats_modulo);

    void record_stats(const ImageMetadata &meta);
};


#endif //SF_DAQ_BUFFER_STREAMSTATS_HPP
