#ifndef SF_DAQ_BUFFER_IMAGEASSEMBLER_HPP
#define SF_DAQ_BUFFER_IMAGEASSEMBLER_HPP


class ImageAssembler {
public:
    void process(const int slot_id,
                 const int i_module,
                 const BufferBinaryBlock* block_buffer);
    int get_free_slot();
    int get_full_slot();
    void free_slot(int slot_id);

    const ImageMetadataBlock* get_metadata_buffer(int slot_id);
    const char* get_data_buffer(int slot_id);

};


#endif //SF_DAQ_BUFFER_IMAGEASSEMBLER_HPP
