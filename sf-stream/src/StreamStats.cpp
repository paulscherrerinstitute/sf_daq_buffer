#include "StreamStats.hpp"

#include <iostream>

using namespace std;
using namespace chrono;

StreamStats::StreamStats(
        const std::string &detector_name,
        const std::string &stream_name,
        const size_t stats_modulo) :
            detector_name_(detector_name),
            stream_name_(stream_name),
            stats_modulo_(stats_modulo)
{
    #ifdef DEBUG_OUTPUT
        using namespace date;
        cout << " [" << std::chrono::system_clock::now();
        cout << "] [StreamStats::StreamStats] ";
        cout << " detector_name: " << detector_name_;
        cout << " || stream_name: " << stream_name;
        cout << " || stats_modulo: " << stats_modulo;
        cout << endl;
    #endif
    reset_counters();
}

void StreamStats::reset_counters()
{
    image_counter_ = 0;
    n_corrupted_images_ = 0;
    stats_interval_start_ = steady_clock::now();
}

void StreamStats::record_stats(
        const ImageMetadata &meta)
{
    image_counter_++;

    if (!meta.is_good_image) {
        n_corrupted_images_++;
    }

    if (image_counter_ == stats_modulo_) {
        print_stats();
        reset_counters();
    }
}

void StreamStats::print_stats()
{
    auto interval_ms_duration = duration_cast<milliseconds>(
            steady_clock::now()-stats_interval_start_).count();
    // * 1000 because milliseconds, + 250 because of truncation.
    int rep_rate = ((image_counter_ * 1000) + 250) / interval_ms_duration;
    uint64_t timestamp = time_point_cast<nanoseconds>(
            system_clock::now()).time_since_epoch().count();

    // Output in InfluxDB line protocol
    cout << "sf_stream";
    cout << ",detector_name=" << detector_name_;
    cout << ",stream_name=" << stream_name_;
    cout << " ";
    cout << "n_processed_images=" << image_counter_ << "i";
    cout << ",n_corrupted_images=" << n_corrupted_images_ << "i";
    cout << ",repetition_rate=" << rep_rate << "i";
    cout << " ";
    cout << timestamp;
    cout << endl;
}

