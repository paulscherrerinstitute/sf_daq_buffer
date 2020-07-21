#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <vector>

#include "zmq.h"
#include "live_writer_config.hpp"
#include "buffer_config.hpp"
#include "bitshuffle/bitshuffle.h"
#include "JFH5LiveWriter.hpp"
#include "LiveImageAssembler.hpp"
#include "BinaryReader.hpp"

using namespace std;
using namespace chrono;
using namespace buffer_config;
using namespace live_writer_config;

void read_buffer(
        const string detector_folder,
        const string module_name,
        const int i_module,
        const vector<uint64_t>& pulse_ids_to_write,
        LiveImageAssembler& image_assembler,
        void* ctx)
{
    BinaryReader reader(detector_folder, module_name);
    auto frame_buffer = new BufferBinaryFormat();

    void* socket = zmq_socket(ctx, ZMQ_SUB);
    if (socket == nullptr) {
        throw runtime_error(zmq_strerror(errno));
    }

    int rcvhwm = 100;
    if (zmq_setsockopt(socket, ZMQ_RCVHWM, &rcvhwm, sizeof(rcvhwm)) != 0) {
        throw runtime_error(zmq_strerror(errno));
    }

    int linger = 0;
    if (zmq_setsockopt(socket, ZMQ_LINGER, &linger, sizeof(linger)) != 0) {
        throw runtime_error(zmq_strerror(errno));
    }

    // In milliseconds.
    int rcvto = 2000;
    if (zmq_setsockopt(socket, ZMQ_RCVTIMEO, &rcvto, sizeof(rcvto)) != 0 ){
        throw runtime_error(zmq_strerror(errno));
    }

    if (zmq_connect(socket, "tcp://127.0.0.1:51234") != 0) {
        throw runtime_error(zmq_strerror(errno));
    }

    if (zmq_setsockopt(socket, ZMQ_SUBSCRIBE, "", 0) != 0) {
        throw runtime_error(zmq_strerror(errno));
    }

    const uint64_t PULSE_ID_DELAY = 100;

    uint64_t live_pulse_id = pulse_ids_to_write.front();
    for (uint64_t pulse_id:pulse_ids_to_write) {

        while(!image_assembler.is_slot_free(pulse_id)) {
            this_thread::sleep_for(chrono::milliseconds(ASSEMBLER_RETRY_MS));
        }

        auto start_time = steady_clock::now();

        // Enforce a delay of 1 second for writing.
        while (live_pulse_id - pulse_id < PULSE_ID_DELAY) {
            if (zmq_recv(socket, &live_pulse_id,
                    sizeof(live_pulse_id), 0) == -1) {
                if (errno == EAGAIN) {
                    throw runtime_error("Did not receive pulse_id in time.");
                } else {
                    throw runtime_error(zmq_strerror(errno));
                }
            }
        }

        reader.get_frame(pulse_id, frame_buffer);

        auto end_time = steady_clock::now();
        uint64_t read_us_duration = duration_cast<microseconds>(
                end_time-start_time).count();

        start_time = steady_clock::now();

        image_assembler.process(pulse_id, i_module, frame_buffer);

        end_time = steady_clock::now();
        uint64_t compose_us_duration = duration_cast<microseconds>(
                end_time-start_time).count();

        cout << "sf_writer:avg_read_us ";
        cout << read_us_duration / BUFFER_BLOCK_SIZE << endl;
        cout << "sf_writer:avg_assemble_us ";
        cout << compose_us_duration / BUFFER_BLOCK_SIZE << endl;
    }

    delete frame_buffer;
}

int main (int argc, char *argv[])
{
    if (argc != 7) {
        cout << endl;
        cout << "Usage: sf_writer [output_file] [detector_folder] [n_modules]";
        cout << " [start_pulse_id] [n_pulses] [pulse_id_step]";
        cout << endl;
        cout << "\toutput_file: Complete path to the output file." << endl;
        cout << "\tdetector_folder: Absolute path to detector buffer." << endl;
        cout << "\tn_modules: number of modules" << endl;
        cout << "\tstart_pulse_id: Start pulse_id of retrieval." << endl;
        cout << "\tn_pulses: Number of pulses to write." << endl;
        cout << "\tpulse_id_step: 1==100Hz, 2==50hz, 4==25Hz.." << endl;
        cout << endl;

        exit(-1);
    }

    string output_file = string(argv[1]);
    const string detector_folder = string(argv[2]);
    size_t n_modules = atoi(argv[3]);
    uint64_t start_pulse_id = (uint64_t) atoll(argv[4]);
    size_t n_pulses = (size_t) atoll(argv[5]);
    int pulse_id_step = atoi(argv[6]);

    std::vector<uint64_t> pulse_ids_to_write;
    uint64_t i_pulse_id = start_pulse_id;
    for (size_t i=0; i<n_pulses; i++) {
        pulse_ids_to_write.push_back(i_pulse_id);
        i_pulse_id += pulse_id_step;
    }

    LiveImageAssembler image_assembler(n_modules);

    auto ctx = zmq_ctx_new();
    zmq_ctx_set (ctx, ZMQ_IO_THREADS, 1);

    std::vector<std::thread> reading_threads(n_modules);
    for (size_t i_module=0; i_module<n_modules; i_module++) {

        // TODO: Very ugly. Fix.
        string module_name = "M";
        if (i_module < 10) {
            module_name += "0";
        }
        module_name += to_string(i_module);

        reading_threads.emplace_back(
                read_buffer,
                detector_folder,
                module_name,
                i_module,
                ref(pulse_ids_to_write),
                ref(image_assembler),
                ctx);
    }

    JFH5LiveWriter writer(output_file, detector_folder, n_modules, n_pulses);

    for (uint64_t pulse_id:pulse_ids_to_write) {

        while(!image_assembler.is_slot_full(pulse_id)) {
            this_thread::sleep_for(chrono::milliseconds(ASSEMBLER_RETRY_MS));
        }

        auto metadata = image_assembler.get_metadata_buffer(pulse_id);
        auto data = image_assembler.get_data_buffer(pulse_id);

        auto start_time = steady_clock::now();

        writer.write(metadata, data);

        auto end_time = steady_clock::now();
        auto write_us_duration = duration_cast<microseconds>(
                end_time-start_time).count();

        image_assembler.free_slot(pulse_id);

        cout << "sf_writer:avg_write_us ";
        cout << write_us_duration / BUFFER_BLOCK_SIZE << endl;
    }

    for (auto& reading_thread : reading_threads) {
        if (reading_thread.joinable()) {
            reading_thread.join();
        }
    }

    return 0;
}
