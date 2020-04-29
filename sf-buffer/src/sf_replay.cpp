#include <iostream>
#include <UdpReceiver.hpp>
#include <thread>
#include "jungfrau.hpp"
#include "BufferUtils.hpp"
#include "zmq.h"
#include "buffer_config.hpp"
#include <H5Cpp.h>
#include "date.h"

using namespace std;
using namespace core_buffer;

struct FileBufferMetadata {
    uint64_t pulse_id[REPLAY_READ_BLOCK_SIZE];
    uint64_t frame_index[REPLAY_READ_BLOCK_SIZE];
    uint32_t daq_rec[REPLAY_READ_BLOCK_SIZE];
    uint16_t n_received_packets[REPLAY_READ_BLOCK_SIZE];
};

void load_data_from_file (
        FileBufferMetadata* metadata_buffer,
        char* image_buffer,
        const string &filename,
        const size_t start_index)
{

    hsize_t b_image_dim[3] = {REPLAY_READ_BLOCK_SIZE, 512, 1024};
    H5::DataSpace b_i_space (3, b_image_dim);
    hsize_t b_i_count[] = {REPLAY_READ_BLOCK_SIZE, 512, 1024};
    hsize_t b_i_start[] = {0, 0, 0};
    b_i_space.selectHyperslab(H5S_SELECT_SET, b_i_count, b_i_start);

    hsize_t f_image_dim[3] = {FILE_MOD, 512, 1024};
    H5::DataSpace f_i_space (3, f_image_dim);
    hsize_t f_i_count[] = {REPLAY_READ_BLOCK_SIZE, 512, 1024};
    hsize_t f_i_start[] = {start_index, 0, 0};
    f_i_space.selectHyperslab(H5S_SELECT_SET, f_i_count, f_i_start);

    hsize_t b_metadata_dim[2] = {REPLAY_READ_BLOCK_SIZE, 1};
    H5::DataSpace b_m_space (2, b_metadata_dim);
    hsize_t b_m_count[] = {REPLAY_READ_BLOCK_SIZE, 1};
    hsize_t b_m_start[] = {0, 0};
    b_m_space.selectHyperslab(H5S_SELECT_SET, b_m_count, b_m_start);

    hsize_t f_metadata_dim[2] = {FILE_MOD, 1};
    H5::DataSpace f_m_space (2, f_metadata_dim);
    hsize_t f_m_count[] = {REPLAY_READ_BLOCK_SIZE, 1};
    hsize_t f_m_start[] = {start_index, 0};
    f_m_space.selectHyperslab(H5S_SELECT_SET, f_m_count, f_m_start);

    H5::H5File input_file(filename, H5F_ACC_RDONLY);

    auto image_dataset = input_file.openDataSet("image");
    image_dataset.read(
            image_buffer, H5::PredType::NATIVE_UINT16,
            b_i_space, f_i_space);

    auto pulse_id_dataset = input_file.openDataSet("pulse_id");
    pulse_id_dataset.read(
            metadata_buffer->pulse_id, H5::PredType::NATIVE_UINT64,
            b_m_space, f_m_space);

    auto frame_id_dataset = input_file.openDataSet("frame_id");
    frame_id_dataset.read(
            metadata_buffer->frame_index, H5::PredType::NATIVE_UINT64,
            b_m_space, f_m_space);

    auto daq_rec_dataset = input_file.openDataSet("daq_rec");
    daq_rec_dataset.read(
            metadata_buffer->daq_rec, H5::PredType::NATIVE_UINT32,
            b_m_space, f_m_space);

    auto received_packets_dataset =
            input_file.openDataSet("received_packets");
    received_packets_dataset.read(
            metadata_buffer->n_received_packets, H5::PredType::NATIVE_UINT16,
            b_m_space, f_m_space);

    input_file.close();
}

void sf_replay (
        void* socket,
        const string& device,
        const string& channel_name,
        const uint16_t module_id,
        const uint64_t start_pulse_id,
        const uint64_t stop_pulse_id)
{
    auto metadata_buffer = make_unique<FileBufferMetadata>();
    auto image_buffer = make_unique<uint16_t[]>(
            REPLAY_READ_BLOCK_SIZE * MODULE_N_PIXELS);

    auto path_suffixes =
            BufferUtils::get_path_suffixes(start_pulse_id, stop_pulse_id);

    uint64_t base_pulse_id = start_pulse_id / core_buffer::FILE_MOD;
    base_pulse_id *= core_buffer::FILE_MOD;

    size_t current_pulse_id = base_pulse_id;
    string filename_base = device + "/" + channel_name + "/";

    for (const auto& filename_suffix:path_suffixes) {

        string filename = filename_base + filename_suffix.path;

        #ifdef DEBUG_OUTPUT
            using namespace date;
            using namespace chrono;

            cout << "[" << system_clock::now() << "]";
            cout << "[sf_replay::sf_replay]";

            cout << " Reading from filename " << filename << endl;
        #endif

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

            for (size_t i_frame=0; i_frame < REPLAY_READ_BLOCK_SIZE; i_frame++) {

                ModuleFrame module_frame = {
                        metadata_buffer->pulse_id[i_frame],
                        metadata_buffer->frame_index[i_frame],
                        metadata_buffer->daq_rec[i_frame],
                        metadata_buffer->n_received_packets[i_frame],
                        module_id
                };

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

                if (current_pulse_id != module_frame.pulse_id) {
                    stringstream err_msg;

                    using namespace date;
                    using namespace chrono;
                    err_msg << "[" << system_clock::now() << "]";
                    err_msg << "[sf_replay::receive]";
                    err_msg << " Read unexpected pulse_id. ";
                    err_msg << " Expected " << current_pulse_id;
                    err_msg << " received " << module_frame.pulse_id;
                    err_msg << endl;

                    throw runtime_error(err_msg.str());
                }

                zmq_send(socket,
                         &module_frame,
                         sizeof(ModuleFrame),
                         ZMQ_SNDMORE);

                auto buff_offset = i_frame * MODULE_N_PIXELS;
                zmq_send(socket,
                         (char*)(image_buffer.get() + buff_offset),
                         MODULE_N_BYTES,
                         0);

                #ifdef DEBUG_OUTPUT
                    using namespace date;
                    using namespace chrono;

                    cout << "[" << system_clock::now() << "]";
                    cout << "[sf_replay::sf_replay]";
                    cout << " Sent pulse_id ";
                    cout << current_pulse_id << endl;
                #endif

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
    const uint16_t module_id = (uint16_t) atoi(argv[3]);
    const uint64_t start_pulse_id = (uint64_t) atoll(argv[4]);
    const uint64_t stop_pulse_id = (uint64_t) atoll(argv[5]);

    stringstream ipc_stream;
    ipc_stream << "ipc://sf-replay-" << (int)module_id;
    const auto ipc_address = ipc_stream.str();

    #ifdef DEBUG_OUTPUT
        using namespace date;
        using namespace chrono;

        cout << "[" << system_clock::now() << "]";
        cout << "[sf_replay::receive]";

        cout << " device " << device;
        cout << " channel_name " << channel_name;
        cout << " module_id " << module_id;
        cout << " start_pulse_id " << start_pulse_id;
        cout << " stop_pulse_id " << stop_pulse_id;
        cout << " ipc_address " << ipc_address;
        cout << endl;
    #endif

    auto ctx = zmq_ctx_new();
    auto socket = zmq_socket(ctx, ZMQ_PUSH);

    const int sndhwm = REPLAY_READ_BLOCK_SIZE;
    if (zmq_setsockopt(socket, ZMQ_SNDHWM, &sndhwm, sizeof(sndhwm)) != 0)
        throw runtime_error(strerror (errno));

    const int linger_ms = 0;
    if (zmq_setsockopt(socket, ZMQ_LINGER, &linger_ms, sizeof(linger_ms)) != 0)
        throw runtime_error(strerror (errno));

    if (zmq_connect(socket, ipc_address.c_str()) != 0)
        throw runtime_error(strerror (errno));

    sf_replay(socket, device, channel_name, module_id,
            start_pulse_id, stop_pulse_id);

    zmq_close(socket);
    zmq_ctx_destroy(ctx);
}
