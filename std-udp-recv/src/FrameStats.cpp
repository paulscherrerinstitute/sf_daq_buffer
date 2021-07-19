#include <iostream>
#include <utility>
#include "FrameStats.hpp"
#include "date.h"
using namespace std;
using namespace chrono;

FrameStats::FrameStats(
        string detector_name,
        const int module_id,
        const int n_packets_per_frame,
        const size_t stats_time) :
            detector_name_(move(detector_name)),
            module_id_(module_id),
            n_packets_per_frame_(n_packets_per_frame),
            stats_time_(stats_time)
{
   reset_counters();
}

void FrameStats::reset_counters()
{
    frames_counter_ = 0;
    n_missed_packets_ = 0;
    n_corrupted_frames_ = 0;
    stats_interval_start_ = steady_clock::now();
}

void FrameStats::record_stats(const ModuleFrame &meta)
{

    if (meta.n_recv_packets < n_packets_per_frame_) {
        n_missed_packets_ += n_packets_per_frame_ - meta.n_recv_packets;
        n_corrupted_frames_++;

        #ifdef DEBUG_OUTPUT
            using namespace date;
            cout << " [" << std::chrono::system_clock::now() << "]";
            cout << " [FrameStats::record_stats] :";
            cout << " meta.pulse_id "<< meta.pulse_id;
            cout << " meta.frame_index "<< meta.frame_index;
            cout << " || meta.n_recv_packets " << meta.n_recv_packets;
            cout << " || n_missed_packets_ " << n_missed_packets_;
            cout << endl;
        #endif
    }

    frames_counter_++;

    const auto time_passed = duration_cast<milliseconds>(
             steady_clock::now()-stats_interval_start_).count(); 

    if (time_passed >= stats_time_*1000) {
        print_stats();
        reset_counters();
    }
}

void FrameStats::print_stats()
{
    auto interval_ms_duration = duration_cast<milliseconds>(
            steady_clock::now()-stats_interval_start_).count();
    // * 1000 because milliseconds, + 250 because of truncation.
    int rep_rate = ((frames_counter_ * 1000) + 250) / interval_ms_duration;
    uint64_t timestamp = time_point_cast<nanoseconds>(
            system_clock::now()).time_since_epoch().count();

    // Output in InfluxDB line protocol
    cout << "std_udp_recv";
    cout << ",detector_name=" << detector_name_;
    cout << ",module_id=" << module_id_;
    cout << " ";
    cout << "n_missed_packets=" << n_missed_packets_ << "i";
    cout << ",n_corrupted_frames=" << n_corrupted_frames_ << "i";
    cout << ",repetition_rate=" << rep_rate << "i";
    cout << " ";
    cout << timestamp;
    cout << endl;
}
