#include <iostream>
#include "FrameStats.hpp"

using namespace std;
using namespace chrono;

FrameStats::FrameStats(const string& source_name, const size_t stats_modulo) :
    source_name_(source_name),
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

    cout << "sf_buffer:device_name " << source_name_;
    cout << " sf_buffer:n_missed_packets " << n_missed_packets_;
    cout << " sf_buffer:n_corrupted_frames " << n_corrupted_frames_;
    cout << " sf_buffer:repetition_rate " << rep_rate;
    cout << endl;
}
