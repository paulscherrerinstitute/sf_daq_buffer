#include "JFH5LiveWriter.hpp"

#include <cstring>
#include <hdf5_hl.h>


#include "buffer_config.hpp"

using namespace std;
using namespace buffer_config;

JFH5LiveWriter::JFH5LiveWriter(const string& output_file,
                       const string& detector_folder,
                       const size_t n_modules,
                       const size_t n_pulses) :
        detector_name_(get_detector_name(detector_folder)),
        n_modules_(n_modules),
        n_pulses_(n_pulses),
        write_index_(0)
{
    b_pulse_id_ = new uint64_t[n_pulses_];
    b_frame_index_= new uint64_t[n_pulses_];
    b_daq_rec_ = new uint32_t[n_pulses_];
    b_is_good_frame_ = new uint8_t[n_pulses_];

    init_file(output_file);
}

void JFH5LiveWriter::init_file(const string& output_file)
{
    file_ = H5::H5File(output_file, H5F_ACC_TRUNC);
    file_.createGroup("/data");
    file_.createGroup("/data/" + detector_name_);

    H5::DataSpace att_space(H5S_SCALAR);
    H5::DataType data_type = H5::StrType(0, H5T_VARIABLE);

    file_.createGroup("/general");
    auto detector_dataset = file_.createDataSet(
            "/general/detector_name", data_type ,att_space);

    detector_dataset.write(detector_name_, data_type);

    hsize_t image_dataset_dims[3] =
            {n_pulses_, n_modules_ * MODULE_Y_SIZE, MODULE_X_SIZE};

    H5::DataSpace image_dataspace(3, image_dataset_dims);

    hsize_t image_dataset_chunking[3] =
            {1, n_modules_ * MODULE_Y_SIZE, MODULE_X_SIZE};
    H5::DSetCreatPropList image_dataset_properties;
    image_dataset_properties.setChunk(3, image_dataset_chunking);

    image_dataset_ = file_.createDataSet(
            "/data/" + detector_name_ + "/data",
            H5::PredType::NATIVE_UINT16,
            image_dataspace,
            image_dataset_properties);
}


std::string JFH5LiveWriter::get_detector_name(const string& detector_folder)
{
    size_t last_separator;
    if ((last_separator = detector_folder.rfind("/")) == string::npos) {
        return detector_folder;
    }

    return detector_folder.substr(last_separator + 1);
}

JFH5LiveWriter::~JFH5LiveWriter()
{
    close_file();

    delete[] b_pulse_id_;
    delete[] b_frame_index_;
    delete[] b_daq_rec_;
    delete[] b_is_good_frame_;
}

void JFH5LiveWriter::write_dataset(
        const string name, const void* buffer, const H5::PredType& type)
{
    hsize_t b_m_dims[] = {n_pulses_};
    H5::DataSpace b_m_space (1, b_m_dims);

    hsize_t f_m_dims[] = {n_pulses_, 1};
    H5::DataSpace f_m_space(2, f_m_dims);

    auto complete_name = "/data/" + detector_name_ + "/" + name;
    auto dataset = file_.createDataSet(complete_name, type, f_m_space);

    dataset.write(buffer, type, b_m_space, f_m_space);

    dataset.close();
}

void JFH5LiveWriter::write_metadata()
{
    write_dataset("pulse_id", &b_pulse_id_, H5::PredType::NATIVE_UINT64);
    write_dataset("frame_index", &b_frame_index_, H5::PredType::NATIVE_UINT64);
    write_dataset("daq_rec", &b_daq_rec_, H5::PredType::NATIVE_UINT32);
    write_dataset("is_good_frame", &b_is_good_frame_, H5::PredType::NATIVE_UINT8);
}

void JFH5LiveWriter::close_file()
{
    if (file_.getId() == -1) {
        return;
    }

    image_dataset_.close();

    write_metadata();

    file_.close();
}

void JFH5LiveWriter::write(const ImageMetadata* metadata, const char* data)
{
    hsize_t offset[] = {write_index_, 0, 0};

    H5DOwrite_chunk(image_dataset_.getId(), H5P_DEFAULT, 0,
            offset, MODULE_N_BYTES * n_modules_, data);

    b_pulse_id_[write_index_] = metadata->pulse_id;
    b_frame_index_[write_index_] = metadata->frame_index;
    b_daq_rec_[write_index_] = metadata->daq_rec;
    b_is_good_frame_[write_index_] = metadata->is_good_image;

    write_index_++;
}
