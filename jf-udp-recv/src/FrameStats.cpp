#include <iostream>
#include "FrameStats.hpp"

using namespace std;
using namespace chrono;

FrameStats::FrameStats(
        const std::string &detector_name,
        const int module_id,
        const size_t stats_modulo) :
            detector_name_(detector_name),
            module_id_(module_id),
            stats_modulo_(stats_modulo)
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
    if (meta.n_recv_packets < JF_N_PACKETS_PER_FRAME) {
        n_missed_packets_ += JF_N_PACKETS_PER_FRAME - meta.n_recv_packets;
        n_corrupted_frames_++;
    }

    frames_counter_++;

    if (frames_counter_ == stats_modulo_) {
        print_stats();
        reset_counters();
    }
}

void FrameStats::print_stats()
{
    auto interval_ms_duration = duration_cast<milliseconds>(
            stats_interval_start_-steady_clock::now()).count();
    // * 1000 because milliseconds, 0.5 for truncation.
    int rep_rate = ((frames_counter_/interval_ms_duration) * 1000) + 0.5;

    uint64_t timestamp = time_point_cast<nanoseconds>(
            system_clock::now()).time_since_epoch().count();

    // Output in InfluxDB line protocol
    cout << "jf-udp-recv";
    cout << ",detector_name=" << detector_name_;
    cout << ",module_name=M" << module_id_;
    cout << " ";
    cout << "n_missed_packets=" << n_missed_packets_ << "i";
    cout << ",n_corrupted_frames=" << n_corrupted_frames_ << "i";
    cout << ",repetition_rate=" << rep_rate << "i";
    cout << " ";
    cout << timestamp;
    cout << endl;
}
