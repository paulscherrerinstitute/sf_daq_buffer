#include <iostream>
#include "BufferStats.hpp"

using namespace std;
using namespace chrono;

BufferStats::BufferStats(
        const string& detector_name,
        const int module_id,
        const size_t stats_modulo) :
            detector_name_(detector_name),
            module_id_(module_id),
            stats_modulo_(stats_modulo)
{
   reset_counters();
}

void BufferStats::reset_counters()
{
    frames_counter_ = 0;
    total_buffer_write_us_ = 0;
    max_buffer_write_us_ = 0;
}

void BufferStats::start_frame_write()
{
    stats_interval_start_ = steady_clock::now();
}

void BufferStats::end_frame_write()
{
    frames_counter_++;

    uint32_t write_us_duration = duration_cast<microseconds>(
            steady_clock::now()-stats_interval_start_).count();

    total_buffer_write_us_ += write_us_duration;
    max_buffer_write_us_ = max(max_buffer_write_us_, write_us_duration);

    if (frames_counter_ == stats_modulo_) {
        print_stats();
        reset_counters();
    }
}

void BufferStats::print_stats()
{
    float avg_buffer_write_us = total_buffer_write_us_ / frames_counter_;

    uint64_t timestamp = time_point_cast<nanoseconds>(
            system_clock::now()).time_since_epoch().count();

    // Output in InfluxDB line protocol
    cout << "jf-buffer-writer";
    cout << ",detector_name=" << detector_name_;
    cout << ",module_name=M" << module_id_;
    cout << " ";
    cout << "avg_buffer_write_us=" << avg_buffer_write_us;
    cout << ",max_buffer_write_us=" << max_buffer_write_us_ << "i";
    cout << " ";
    cout << timestamp;
    cout << endl;
}
