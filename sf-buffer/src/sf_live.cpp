#include <iostream>
#include <thread>
#include "jungfrau.hpp"
#include "BufferUtils.hpp"
#include "zmq.h"
#include "buffer_config.hpp"
#include <H5Cpp.h>
#include <cstring>
#include "date.h"
#include "LiveH5Reader.hpp"

using namespace std;
using namespace core_buffer;

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

void sf_live (
        void* socket,
        const string& device,
        const string& channel_name,
        const uint16_t source_id)
{
    auto metadata_buffer = make_unique<ModuleFrame>();
    auto image_buffer = make_unique<uint16_t[]>(MODULE_N_PIXELS);

    const auto current_filename = device + "/" + channel_name + "/CURRENT";

    LiveH5Reader reader(current_filename, source_id);

    auto current_pulse_id = reader.get_latest_pulse_id();

    while (true) {

        reader.get_frame_metadata(current_pulse_id, metadata_buffer.get());

        zmq_send(socket,
                 (char*)(metadata_buffer.get()),
                 sizeof(ModuleFrame),
                 ZMQ_SNDMORE);

        reader.get_frame_data(current_pulse_id, image_buffer.get());

        zmq_send(socket,
                 (char*)(image_buffer.get()),
                 MODULE_N_BYTES,
                 0);

        #ifdef DEBUG_OUTPUT
            using namespace date;
            using namespace chrono;

            cout << "[" << system_clock::now() << "]";
            cout << "[sf_live::sf_live]";
            cout << " Sent pulse_id ";
            cout << current_pulse_id << endl;
        #endif

        current_pulse_id++;
    }
}

int main (int argc, char *argv[]) {

    if (argc != 6) {
        cout << endl;
        cout << "Usage: sf_live [device] [channel_name] [source_id]";
        cout << endl;
        cout << "\tdevice: Name of detector." << endl;
        cout << "\tchannel_name: M00-M31 for JF16M." << endl;
        cout << "\tsource_id: Module index" << endl;
        cout << endl;

        exit(-1);
    }

    const string device = string(argv[1]);
    const string channel_name = string(argv[2]);
    const uint16_t source_id = (uint16_t) atoi(argv[3]);

    stringstream ipc_stream;
    ipc_stream << "ipc://sf-live-" << (int)source_id;
    const auto ipc_address = ipc_stream.str();

    #ifdef DEBUG_OUTPUT
        using namespace date;
        using namespace chrono;

        cout << "[" << system_clock::now() << "]";
        cout << "[sf_live::main]";

        cout << " device " << device;
        cout << " channel_name " << channel_name;
        cout << " source_id " << source_id;
        cout << " start_pulse_id " << start_pulse_id;
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

    sf_live(socket, device, channel_name, source_id);

    zmq_close(socket);
    zmq_ctx_destroy(ctx);
}
