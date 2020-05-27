#ifndef BINARYWRITER_HPP
#define BINARYWRITER_HPP

#include <string>

#include "formats.hpp"

class BufferBinaryWriter {

    const std::string root_folder_;
    const std::string device_name_;
    std::string latest_filename_;

    std::string current_output_filename_;
    int output_file_fd_;

    void open_file(const std::string& filename);
    void close_current_file();


public:
    BufferBinaryWriter(
            const std::string& root_folder,
            const std::string& device_name);

    virtual ~BufferBinaryWriter();

    void write(const uint64_t pulse_id, const BufferBinaryFormat* buffer);

};


#endif //BINARYWRITER_HPP
