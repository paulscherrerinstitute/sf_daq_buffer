#include <BufferUtils.hpp>
#include "BufferH5Writer.hpp"
#include <chrono>
#include <WriterUtils.hpp>
#include <cstring>
#include <buffer_config.hpp>

extern "C"
{
    #include "H5DOpublic.h"
}

using namespace std;
using namespace core_buffer;

BufferH5Writer::BufferH5Writer(
        const string& device_name,
        const string& root_folder) :
            device_name_(device_name),
            root_folder_(root_folder),
            LATEST_filename_(root_folder + "/" + device_name + "/LATEST"),
            CURRENT_filename_(root_folder + "/" + device_name + "/CURRENT"),
            output_filename_(""),
            current_pulse_id_(0),
            current_file_index_(0)
{
}

void BufferH5Writer::create_file(const string& filename)
{
    {
        h5_file_ = H5::H5File(filename, H5F_ACC_TRUNC | H5F_ACC_SWMR_WRITE);

        output_filename_ = filename;

        H5::DataSpace data_dspace(3, data_disk_dims, data_disk_dims);
        H5::DSetCreatPropList data_dset_prop;
        hsize_t data_dset_chunking[3] = {1, MODULE_Y_SIZE, MODULE_X_SIZE};
        data_dset_prop.setChunk(3, data_dset_chunking);

        h5_file_.createDataSet(
                "image",
                H5::PredType::NATIVE_UINT16,
                data_dspace,
                data_dset_prop);

        H5::DataSpace meta_dspace(2, meta_disk_dims, meta_disk_dims);
        H5::DSetCreatPropList meta_dset_prop;
        hsize_t meta_dset_chunking[2] = {1, ModuleFrame_N_FIELDS};
        meta_dset_prop.setChunk(2, meta_dset_chunking);

        h5_file_.createDataSet(
                "metadata",
                H5::PredType::NATIVE_UINT64,
                meta_dspace,
                meta_dset_prop);

        h5_file_.close();
    }

    h5_file_ = H5::H5File(filename, H5F_ACC_RDWR |H5F_ACC_SWMR_WRITE);

    current_image_dataset_ = h5_file_.openDataSet("image");
    current_metadata_dataset_ = h5_file_.openDataSet("metadata");
}

BufferH5Writer::~BufferH5Writer()
{
    close_file();
}

void BufferH5Writer::close_file() {
    current_image_dataset_.close();
    current_metadata_dataset_.close();

    h5_file_.close();
    output_filename_ = "";

    current_pulse_id_ = 0;
    current_file_index_ = 0;
}

void BufferH5Writer::set_pulse_id(const uint64_t pulse_id)
{
    current_pulse_id_ = pulse_id;
    current_file_index_ = BufferUtils::get_file_frame_index(pulse_id);

    auto new_output_filename = BufferUtils::get_filename(
            root_folder_, device_name_, pulse_id);

    if (new_output_filename != output_filename_){

        if (h5_file_.getId() != -1) {
            auto latest_filename = output_filename_;
            close_file();
            BufferUtils::update_latest_file(LATEST_filename_, latest_filename);
        }

        WriterUtils::create_destination_folder(new_output_filename);
        create_file(new_output_filename);

        BufferUtils::update_latest_file(CURRENT_filename_, output_filename_);
    }
}

void BufferH5Writer::write(const ModuleFrame* metadata, const char* data)
{
    hsize_t meta_buff_dims[1] = {ModuleFrame_N_FIELDS};
    H5::DataSpace meta_buffer_space (1, meta_buff_dims);

    H5::DataSpace meta_disk_space(2, meta_disk_dims);
    hsize_t meta_count[] = {1, ModuleFrame_N_FIELDS};
    hsize_t meta_start[] = {current_file_index_, 0};
    meta_disk_space.selectHyperslab(H5S_SELECT_SET, meta_count, meta_start);

    current_metadata_dataset_.write(
            (char*) metadata,
            H5::PredType::NATIVE_UINT64,
            meta_buffer_space,
            meta_disk_space);

    hsize_t data_buff_dims[2] = {MODULE_Y_SIZE, MODULE_X_SIZE};
    H5::DataSpace data_buffer_space (2, data_buff_dims);

    H5::DataSpace data_disk_space(3, data_disk_dims);
    hsize_t data_count[] = {1, MODULE_Y_SIZE, MODULE_X_SIZE};
    hsize_t data_start[] = {current_file_index_, 0, 0};
    data_disk_space.selectHyperslab(H5S_SELECT_SET, data_count, data_start);

    current_image_dataset_.write(
            data,
            H5::PredType::NATIVE_UINT16,
            data_buffer_space,
            data_disk_space);

    H5Dflush(current_metadata_dataset_.getId());
    H5Dflush(current_image_dataset_.getId());
}
