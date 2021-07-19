#include "AssemblerStats.hpp"

#include <iostream>

using namespace std;
using namespace chrono;

AssemblerStats::AssemblerStats(
        const std::string &detector_name,
        const size_t stats_time) :
            detector_name_(detector_name),
            stats_time_(stats_time)
{
    reset_counters();
}

void AssemblerStats::reset_counters()
{
    image_counter_ = 0;
    n_sync_lost_images_ = 0;
    n_corrupted_images_ = 0;
    stats_interval_start_ = steady_clock::now();
}

void AssemblerStats::record_stats(
        const ImageMetadata *meta, const uint32_t n_lost_pulses)
{
    image_counter_++;
    n_sync_lost_images_ += n_lost_pulses;

    if (meta->status != 0) {
        n_corrupted_images_++;
    }

    const auto time_passed = duration_cast<milliseconds>(
             steady_clock::now()-stats_interval_start_).count();

    if (time_passed >= stats_time_*1000) {
        print_stats();
        reset_counters();
    }
}

void AssemblerStats::print_stats()
{
    auto interval_ms_duration = duration_cast<milliseconds>(
            steady_clock::now()-stats_interval_start_).count();
    // * 1000 because milliseconds, + 250 because of truncation.
    int rep_rate = ((image_counter_ * 1000) + 250) / interval_ms_duration;
    uint64_t timestamp = time_point_cast<nanoseconds>(
            system_clock::now()).time_since_epoch().count();

    // Output in InfluxDB line protocol
    cout << "jf_assembler";
    cout << ",detector_name=" << detector_name_;
    cout << " ";
    cout << "n_processed_images=" << image_counter_ << "i";
    cout << ",n_corrupted_images=" << n_corrupted_images_ << "i";
    cout << ",n_sync_lost_images=" << n_sync_lost_images_ << "i";
    cout << ",repetition_rate=" << rep_rate << "i";
    cout << " ";
    cout << timestamp;
    cout << endl;
}

