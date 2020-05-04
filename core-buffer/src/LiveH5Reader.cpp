#include "LiveH5Reader.hpp"
#include "BufferUtils.hpp"

using namespace std;
using namespace core_buffer;

LiveH5Reader::LiveH5Reader(
        const std::string& device,
        const std::string& channel_name,
        const uint16_t source_id):
            current_filename_(device + "/" + channel_name + "/CURRENT"),
            source_id_(source_id),
            pulse_id_buffer_(make_unique<uint64_t[]>(FILE_MOD)),
            data_buffer_(make_unique<uint16_t[]>(MODULE_N_PIXELS))
{
//    auto filename = BufferUtils::get_latest_file(current_filename_);
//    file_ = H5::H5File(filename, H5F_ACC_RDONLY |  H5F_ACC_SWMR_READ);
//
//    uint64_t base_pulse_id = start_pulse_id / core_buffer::FILE_MOD;
//    base_pulse_id *= core_buffer::FILE_MOD;
//
//    current_file_max_pulse_id_ =
//
//    image_dataset_ = input_file.openDataSet("image");
//    pulse_id_dataset_ = input_file.openDataSet("pulse_id");
//    frame_index_dataset_ = input_file.openDataSet("frame_id");
//    daq_rec_dataset_ = input_file.openDataSet("daq_rec");
//    n_received_packets_dataset_ = input_file.openDataSet("received_packets");

}

LiveH5Reader::~LiveH5Reader() {
    close_file();
}



//void load_data_from_file (
//        FileBufferMetadata* metadata_buffer,
//        char* image_buffer,
//        const string &filename,
//        const size_t start_index)
//{
//
//    hsize_t b_image_dim[3] = {REPLAY_READ_BLOCK_SIZE, 512, 1024};
//    H5::DataSpace b_i_space (3, b_image_dim);
//    hsize_t b_i_count[] = {REPLAY_READ_BLOCK_SIZE, 512, 1024};
//    hsize_t b_i_start[] = {0, 0, 0};
//    b_i_space.selectHyperslab(H5S_SELECT_SET, b_i_count, b_i_start);
//
//    hsize_t f_image_dim[3] = {FILE_MOD, 512, 1024};
//    H5::DataSpace f_i_space (3, f_image_dim);
//    hsize_t f_i_count[] = {REPLAY_READ_BLOCK_SIZE, 512, 1024};
//    hsize_t f_i_start[] = {start_index, 0, 0};
//    f_i_space.selectHyperslab(H5S_SELECT_SET, f_i_count, f_i_start);
//
//    hsize_t b_metadata_dim[2] = {REPLAY_READ_BLOCK_SIZE, 1};
//    H5::DataSpace b_m_space (2, b_metadata_dim);
//    hsize_t b_m_count[] = {REPLAY_READ_BLOCK_SIZE, 1};
//    hsize_t b_m_start[] = {0, 0};
//    b_m_space.selectHyperslab(H5S_SELECT_SET, b_m_count, b_m_start);
//
//    hsize_t f_metadata_dim[2] = {FILE_MOD, 1};
//    H5::DataSpace f_m_space (2, f_metadata_dim);
//    hsize_t f_m_count[] = {REPLAY_READ_BLOCK_SIZE, 1};
//    hsize_t f_m_start[] = {start_index, 0};
//    f_m_space.selectHyperslab(H5S_SELECT_SET, f_m_count, f_m_start);
//
//    H5::H5File input_file(filename, H5F_ACC_RDONLY);
//
//    auto image_dataset = input_file.openDataSet("image");
//    image_dataset.read(
//            image_buffer, H5::PredType::NATIVE_UINT16,
//            b_i_space, f_i_space);
//
//    auto pulse_id_dataset = input_file.openDataSet("pulse_id");
//    pulse_id_dataset.read(
//            metadata_buffer->pulse_id, H5::PredType::NATIVE_UINT64,
//            b_m_space, f_m_space);
//
//    auto frame_id_dataset = input_file.openDataSet("frame_id");
//    frame_id_dataset.read(
//            metadata_buffer->frame_index, H5::PredType::NATIVE_UINT64,
//            b_m_space, f_m_space);
//
//    auto daq_rec_dataset = input_file.openDataSet("daq_rec");
//    daq_rec_dataset.read(
//            metadata_buffer->daq_rec, H5::PredType::NATIVE_UINT32,
//            b_m_space, f_m_space);
//
//    auto received_packets_dataset =
//            input_file.openDataSet("received_packets");
//    received_packets_dataset.read(
//            metadata_buffer->n_received_packets, H5::PredType::NATIVE_UINT16,
//            b_m_space, f_m_space);
//
//    input_file.close();
//}