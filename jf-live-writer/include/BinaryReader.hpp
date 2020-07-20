#ifndef SF_DAQ_BUFFER_BINARYREADER_HPP
#define SF_DAQ_BUFFER_BINARYREADER_HPP


#include <formats.hpp>

class BinaryReader {

    const std::string root_folder_;
    const std::string device_name_;

    std::string current_input_file_;
    int input_file_fd_;

    void open_file(const std::string& filename);
    void close_current_file();

public:
    BinaryReader(const std::string &root_folder,
                 const std::string &device_name);

    ~BinaryReader();

    void get_frame(const uint64_t pulse_id, BufferBinaryFormat *buffer);
};


#endif //SF_DAQ_BUFFER_BINARYREADER_HPP
