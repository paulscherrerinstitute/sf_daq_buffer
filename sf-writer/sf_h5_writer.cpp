#include <iostream>
#include <stdexcept>
#include <H5Writer.hpp>
#include "BufferMultiReader.hpp"

#include "config.hpp"
#include "SfFormat.cpp"

using namespace std;

int main (int argc, char *argv[])
{
    if (argc != 5) {
        cout << endl;
        cout << "Usage: sf_h5_writer [root_folder]";
        cout << " [output_file] [start_pulse_id] [stop_pulse_id]";
        cout << endl;
        cout << "\troot_folder: Base of the buffer." << endl;
        cout << "\toutput_file: Complete path to the output file." << endl;
        cout << "\tstart_pulse_id: Start pulse_id of retrieval." << endl;
        cout << "\tstop_pulse_id: Stop pulse_id of retrieval." << endl;
        cout << endl;

        exit(-1);
    }

    string root_folder = string(argv[1]);
    string output_file = string(argv[2]);
    uint64_t start_pulse_id = (uint64_t) atoi(argv[3]);
    uint64_t stop_pulse_id = (uint64_t) atoi(argv[4]);

    BufferMultiReader reader(root_folder);
    UdpFrameMetadata metadata;
    char* frame_buffer = reader.get_buffer();

    H5Writer writer(output_file);

    size_t output_file_position = 0;

    for (size_t pulse_id=start_pulse_id;
            pulse_id <= stop_pulse_id;
            pulse_id++) {

        metadata = reader.load_frame_to_buffer(pulse_id);

        writer.write_data("frame", output_file_position,
                          frame_buffer,
                          {32*512, 1024}, 32*512*1024*2, "uint16", "little");

        writer.write_data("pulse_id", output_file_position,
                          (char*)&(metadata.pulse_id),
                          {1}, 8, "uint64", "little");

        writer.write_data("frame", output_file_position,
                          (char*)&(metadata.frame_index),
                          {1}, 8, "uint64", "little");

        writer.write_data("daq_rec", output_file_position,
                          (char*)&(metadata.daq_rec),
                          {1}, 4, "uint32", "little");

        uint64_t is_good_frame = 0;
        if (metadata.n_recv_packets == 4096) {
            is_good_frame = 1;
        }

        writer.write_data("is_good_frame", output_file_position,
                          (char*)&(is_good_frame),
                          {1}, 8, "uint64", "little");

        output_file_position++;
    }

    return 0;
}
