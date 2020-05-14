#include "ReplayH5Reader.hpp"

#include "BufferUtils.hpp"
#include "buffer_config.hpp"

using namespace std;
using namespace core_buffer;

void ReplayH5Reader::prepare_file_for_pulse(const uint64_t pulse_id)
{
    auto pulse_filename = BufferUtils::get_filename(
            device_, channel_name_, pulse_id);

    if (pulse_filename == current_filename_) {
        return;
    }

    close_file();

    current_filename_ = pulse_filename;
    current_file_ = H5::H5File(current_filename_, H5F_ACC_RDONLY);

    dset_metadata_ = current_file_.openDataSet(BUFFER_H5_METADATA_DATASET);
    dset_frame_ = current_file_.openDataSet(BUFFER_H5_FRAME_DATASET);
}

ReplayH5Reader::ReplayH5Reader(
        const string device,
        const string channel_name) :
            device_(device),
            channel_name_(channel_name)
{
}

ReplayH5Reader::~ReplayH5Reader()
{
    close_file();
}

void ReplayH5Reader::close_file()
{
    if (current_file_.getId() != -1) {
        dset_metadata_.close();
        dset_frame_.close();
        current_file_.close();
    }
}

void ReplayH5Reader::get_frame(
        const uint64_t pulse_id, ModuleFrame* metadata, char* data)
{
    prepare_file_for_pulse(pulse_id);

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

    metadata_dataset.read(
            (char*) metadata_buffer, H5::PredType::NATIVE_UINT64,
            b_m_space, f_m_space);


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

    current_pulse_id++;

}