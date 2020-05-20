#ifndef SF_DAQ_BUFFER_BUFFEREDFASTQUEUE_HPP
#define SF_DAQ_BUFFER_BUFFEREDFASTQUEUE_HPP

#include "FastQueue.hpp"
#include "WriterH5Writer.hpp"


class BufferedFastQueue {
    FastQueue<ImageMetadataBuffer>& queue_;
    const size_t buffer_n_pulses_;
    const size_t n_modules_;

    ImageMetadataBuffer* queue_meta_buffer_ = nullptr;
    char* queue_data_buffer_ = nullptr;
    int current_slot_id_ = -1;

    ImageMetadata image_metadata_;

public:
    BufferedFastQueue(FastQueue<ImageMetadataBuffer>& queue,
                      const size_t buffer_n_pulses,
                      const size_t n_modules);

    ImageMetadata* get_metadata_buffer();
    char* get_data_buffer();

    void commit();
    void finalize();
};


#endif //SF_DAQ_BUFFER_BUFFEREDFASTQUEUE_HPP
