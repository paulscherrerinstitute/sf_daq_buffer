#include <iostream>
#include <thread>
#include "jungfrau.hpp"
#include "BufferUtils.hpp"
#include "zmq.h"
#include "buffer_config.hpp"
#include <H5Cpp.h>
#include <cstring>
#include "date.h"

using namespace std;
using namespace core_buffer;

void load_data_from_file (
        ModuleFrame* metadata_buffer,
        char* image_buffer,
        const string &filename,
        const size_t start_index)
{
    H5::H5File input_file(filename, H5F_ACC_RDONLY);

    hsize_t b_image_dims[3] =
            {REPLAY_READ_BLOCK_SIZE, MODULE_Y_SIZE, MODULE_X_SIZE};
    H5::DataSpace b_i_space (3, b_image_dims);
    hsize_t b_i_count[] =
            {REPLAY_READ_BLOCK_SIZE, MODULE_Y_SIZE, MODULE_X_SIZE};
    hsize_t b_i_start[] = {0, 0, 0};
    b_i_space.selectHyperslab(H5S_SELECT_SET, b_i_count, b_i_start);

    hsize_t f_image_dims[3] = {FILE_MOD, MODULE_Y_SIZE, MODULE_X_SIZE};
    H5::DataSpace f_i_space (3, f_image_dims);
    hsize_t f_i_count[] =
            {REPLAY_READ_BLOCK_SIZE, MODULE_Y_SIZE, MODULE_X_SIZE};
    hsize_t f_i_start[] = {start_index, 0, 0};
    f_i_space.selectHyperslab(H5S_SELECT_SET, f_i_count, f_i_start);

    auto image_dataset = input_file.openDataSet("image");
    image_dataset.read(
            image_buffer, H5::PredType::NATIVE_UINT16,
            b_i_space, f_i_space);

    hsize_t b_metadata_dims[2] = {REPLAY_READ_BLOCK_SIZE, ModuleFrame_N_FIELDS};
    H5::DataSpace b_m_space (2, b_metadata_dims);
    hsize_t b_m_count[] = {REPLAY_READ_BLOCK_SIZE, ModuleFrame_N_FIELDS};
    hsize_t b_m_start[] = {0, 0};
    b_m_space.selectHyperslab(H5S_SELECT_SET, b_m_count, b_m_start);

    hsize_t f_metadata_dims[2] = {FILE_MOD, ModuleFrame_N_FIELDS};
    H5::DataSpace f_m_space (2, f_metadata_dims);
    hsize_t f_m_count[] = {REPLAY_READ_BLOCK_SIZE, ModuleFrame_N_FIELDS};
    hsize_t f_m_start[] = {start_index, 0};
    f_m_space.selectHyperslab(H5S_SELECT_SET, f_m_count, f_m_start);

    auto metadata_dataset = input_file.openDataSet("metadata");
    metadata_dataset.read(
            (char*) metadata_buffer, H5::PredType::NATIVE_UINT64,
            b_m_space, f_m_space);

    input_file.close();
}

void sf_replay (
        void* socket,
        const string& device,
        const string& channel_name,
        const uint64_t start_pulse_id,
        const uint64_t stop_pulse_id)
{
    auto metadata_buffer = make_unique<ModuleFrame[]>(REPLAY_READ_BLOCK_SIZE);
    auto image_buffer = make_unique<uint16_t[]>(
            REPLAY_READ_BLOCK_SIZE * MODULE_N_PIXELS);

    auto path_suffixes =
            BufferUtils::get_path_suffixes(start_pulse_id, stop_pulse_id);

    uint64_t base_pulse_id = start_pulse_id / core_buffer::FILE_MOD;
    base_pulse_id *= core_buffer::FILE_MOD;

    size_t current_pulse_id = base_pulse_id;
    string filename_base = core_buffer::BUFFER_BASE_DIR + "/" + device + "/" + channel_name + "/";

    for (const auto& filename_suffix:path_suffixes) {

        string filename = filename_base + filename_suffix.path;

        for (size_t file_index_offset=0;
             file_index_offset < FILE_MOD;
             file_index_offset += REPLAY_READ_BLOCK_SIZE)
        {
            auto start_time = chrono::steady_clock::now();

            load_data_from_file(
                    metadata_buffer.get(),
                    (char*)(image_buffer.get()),
                    filename,
                    file_index_offset);

            auto end_time = chrono::steady_clock::now();
            auto ms_duration = chrono::duration_cast<chrono::milliseconds>(
                    end_time-start_time).count();

            cout << "sf_replay:batch_read_ms " << ms_duration << endl;

            for (
                    size_t i_frame=0;
                    i_frame < REPLAY_READ_BLOCK_SIZE;
                    i_frame++) {

                auto current_frame = (metadata_buffer.get())[i_frame];

                if (current_pulse_id < start_pulse_id) {
                    current_pulse_id++;
                    continue;
                }

                if (current_pulse_id > stop_pulse_id) {
                    cout << "Done. Streamed images from ";
                    cout << start_pulse_id << " to " << stop_pulse_id;
                    cout << endl;
                    return;
                }

                // The buffer did not write this pulse id.
                if (current_frame.pulse_id == 0) {
                    cout << "pulse_id " << current_pulse_id;
                    cout << " missing in buffer file." << endl;
                // Wrong frame in the buffer file.
                } else if (current_pulse_id != current_frame.pulse_id) {
                    stringstream err_msg;

                    using namespace date;
                    using namespace chrono;
                    err_msg << "[" << system_clock::now() << "]";
                    err_msg << "[sf_replay::receive]";
                    err_msg << " Read unexpected pulse_id. ";
                    err_msg << " Expected " << current_pulse_id;
                    err_msg << " received " << current_frame.pulse_id;
                    err_msg << endl;

                    throw runtime_error(err_msg.str());
                }

                zmq_send(socket,
                         &current_frame,
                         sizeof(ModuleFrame),
                         ZMQ_SNDMORE);

                auto buff_offset = i_frame * MODULE_N_PIXELS;
                zmq_send(socket,
                         (char*)(image_buffer.get() + buff_offset),
                         MODULE_N_BYTES,
                         0);

                current_pulse_id++;
            }
        }
    }
}

int main (int argc, char *argv[]) {

    if (argc != 6) {
        cout << endl;
        cout << "Usage: sf_replay [device]";
        cout << " [channel_name] [module_id] [start_pulse_id] [stop_pulse_id]";
        cout << endl;
        cout << "\tdevice: Name of detector." << endl;
        cout << "\tchannel_name: M00-M31 for JF16M." << endl;
        cout << "\tmodule_id: Module index" << endl;
        cout << "\tstart_pulse_id: Start pulse_id of retrieval." << endl;
        cout << "\tstop_pulse_id: Stop pulse_id of retrieval." << endl;
        cout << endl;

        exit(-1);
    }

    const string device = string(argv[1]);
    const string channel_name = string(argv[2]);
    const auto module_id = (uint16_t) atoi(argv[3]);
    const auto start_pulse_id = (uint64_t) atoll(argv[4]);
    const auto stop_pulse_id = (uint64_t) atoll(argv[5]);

    stringstream ipc_stream;
    ipc_stream << "ipc:///tmp/sf-replay-" << (int)module_id;
    const auto ipc_address = ipc_stream.str();

    auto ctx = zmq_ctx_new();
    auto socket = zmq_socket(ctx, ZMQ_PUSH);

    const int sndhwm = REPLAY_READ_BLOCK_SIZE;
    if (zmq_setsockopt(socket, ZMQ_SNDHWM, &sndhwm, sizeof(sndhwm)) != 0)
        throw runtime_error(strerror (errno));

    const int linger_ms = -1;
    if (zmq_setsockopt(socket, ZMQ_LINGER, &linger_ms, sizeof(linger_ms)) != 0)
        throw runtime_error(strerror (errno));

    if (zmq_connect(socket, ipc_address.c_str()) != 0)
        throw runtime_error(strerror (errno));

    sf_replay(socket, device, channel_name, start_pulse_id, stop_pulse_id);

    zmq_close(socket);
    zmq_ctx_destroy(ctx);
}
