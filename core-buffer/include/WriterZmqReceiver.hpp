#ifndef SF_DAQ_BUFFER_WRITERZMQRECEIVER_HPP
#define SF_DAQ_BUFFER_WRITERZMQRECEIVER_HPP


class WriterZmqReceiver {
public:
    WriterZmqReceiver(
            void *ctx,
            const std::string& ipc_prefix,
            const size_t n_modules);

    void get_next_image(
            ImageMetadata* image_metadata,
            char* image_buffer);
};


#endif //SF_DAQ_BUFFER_WRITERZMQRECEIVER_HPP
