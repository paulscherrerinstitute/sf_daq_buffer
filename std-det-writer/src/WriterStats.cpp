#include <iostream>
#include <utility>
#include "WriterStats.hpp"

using namespace std;
using namespace chrono;

WriterStats::WriterStats(string detector_name, size_t image_n_bytes) :
    detector_name_(std::move(detector_name)),
    image_n_bytes_(image_n_bytes)
{
   reset_counters();
}

void WriterStats::reset_counters()
{
    image_counter_ = 0;
    total_buffer_write_us_ = 0;
    max_buffer_write_us_ = 0;
    total_bytes_ = 0;
}

void WriterStats::start_image_write()
{
    stats_interval_start_ = steady_clock::now();
}

void WriterStats::end_image_write()
{
    image_counter_++;
    total_bytes_ += image_n_bytes_;

    uint32_t write_us_duration = duration_cast<microseconds>(
            steady_clock::now()-stats_interval_start_).count();

    total_buffer_write_us_ += write_us_duration;
    max_buffer_write_us_ = max(max_buffer_write_us_, write_us_duration);
}

void WriterStats::end_run()
{
    print_stats();
    reset_counters();
}

void WriterStats::print_stats()
{
    float avg_buffer_write_us = 0;
    uint64_t avg_throughput = 0;
    if (image_counter_ > 0) {
        avg_buffer_write_us = total_buffer_write_us_ / image_counter_;
        avg_throughput =
            // bytes -> megabytes
            (image_n_bytes_ / 1024 / 1024) /
            // micro seconds -> seconds
            (avg_buffer_write_us * 1000 * 1000);
    }

    const uint64_t timestamp = time_point_cast<nanoseconds>(
            system_clock::now()).time_since_epoch().count();    

    // Output in InfluxDB line protocol
    cout << "jf_buffer_writer";
    cout << ",detector_name=" << detector_name_;
    cout << " ";
    cout << "n_written_images=" << image_counter_ << "i";
    cout << " ,avg_buffer_write_us=" << avg_buffer_write_us;
    cout << " ,max_buffer_write_us=" << max_buffer_write_us_ << "i";
    cout << " ,avg_throughput=" << avg_throughput;
    cout << timestamp;
    cout << endl;
}
