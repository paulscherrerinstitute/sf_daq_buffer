#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <vector>

#include "date.h"
#include "zmq.h"
#include "writer_config.hpp"
#include "buffer_config.hpp"
#include "bitshuffle/bitshuffle.h"
#include "JFH5Writer.hpp"
#include "ImageAssembler.hpp"
#include "BinaryReader.hpp"

using namespace std;
using namespace chrono;
using namespace writer_config;
using namespace buffer_config;

void read_buffer(
        const string detector_folder,
        const string module_name,
        const int i_module,
        const vector<uint64_t>& pulse_ids_to_write,
        ImageAssembler& image_assembler)
{
    BinaryReader reader(detector_folder, module_name);
    auto frame_buffer = new BufferBinaryFormat();

    for (uint64_t pulse_id:pulse_ids_to_write) {

        while(!image_assembler.is_slot_free(pulse_id)) {
            this_thread::sleep_for(chrono::milliseconds(ASSEMBLER_RETRY_MS));
        }

        auto start_time = steady_clock::now();

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
        cout << " [start_pulse_id] [stop_pulse_id] [pulse_id_step]";
        cout << endl;
        cout << "\toutput_file: Complete path to the output file." << endl;
        cout << "\tdetector_folder: Absolute path to detector buffer." << endl;
        cout << "\tn_modules: number of modules" << endl;
        cout << "\tstart_pulse_id: Start pulse_id of retrieval." << endl;
        cout << "\tstop_pulse_id: Stop pulse_id of retrieval." << endl;
        cout << "\tpulse_id_step: 1==100Hz, 2==50hz, 4==25Hz.." << endl;
        cout << endl;

        exit(-1);
    }

    string output_file = string(argv[1]);
    const string detector_folder = string(argv[2]);
    size_t n_modules = atoi(argv[3]);
    uint64_t start_pulse_id = (uint64_t) atoll(argv[4]);
    uint64_t stop_pulse_id = (uint64_t) atoll(argv[5]);
    int pulse_id_step = atoi(argv[6]);

    std::vector<uint64_t> pulse_ids_to_write;
    for (uint64_t curr_pulse_id=start_pulse_id;
         curr_pulse_id <= stop_pulse_id;
         curr_pulse_id+= pulse_id_step) {
        pulse_ids_to_write.push_back(curr_pulse_id);
    }

    ImageAssembler image_assembler(n_modules);

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
                ref(image_assembler));
    }

    JFH5Writer writer(output_file, detector_folder, n_modules,
                      start_pulse_id, stop_pulse_id, pulse_id_step);

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
