#ifndef SF_DAQ_BUFFER_IMAGEASSEMBLER_HPP
#define SF_DAQ_BUFFER_IMAGEASSEMBLER_HPP


class ImageAssembler {
public:
    ImageAssembler();
    ImageMetadataBlock* get_metadata_buffer(const int slot_id);
    virtual ~ImageAssembler();

    void process(const int slot_id,
                 const int i_module,
                 const BufferBinaryBlock* block_buffer);
    int get_free_slot();
    int get_full_slot();
    void free_slot(const int slot_id);


    const char* get_data_buffer(const int slot_id);
};


#endif //SF_DAQ_BUFFER_IMAGEASSEMBLER_HPP
