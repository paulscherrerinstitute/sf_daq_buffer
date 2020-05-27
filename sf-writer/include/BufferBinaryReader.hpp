#ifndef SF_DAQ_BUFFER_BUFFERBINARYREADER_HPP
#define SF_DAQ_BUFFER_BUFFERBINARYREADER_HPP


#include <formats.hpp>

class BufferBinaryReader {

    const std::string device_;
    const std::string channel_name_;

    std::string current_input_file_;
    int input_file_fd_;

    void open_file(const std::string& filename);
    void close_current_file();

public:
    BufferBinaryReader(const std::string &device,
                       const std::string &channel_name);

    virtual ~BufferBinaryReader();

    void get_block(const uint64_t block_id, BufferBinaryBlock *buffer);
};


#endif //SF_DAQ_BUFFER_BUFFERBINARYREADER_HPP
