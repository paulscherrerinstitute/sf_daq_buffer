#ifndef FRAME_CACHE_HPP
#define FRAME_CACHE_HPP

#include <cstddef>
#include <stdexcept>
#include <iostream>
#include <mutex>
#include <vector>
#include <atomic>
#include <functional>
#include <thread>

#include "jungfraujoch.hpp"


/** Frame cache

	Reimplemented RamBuffer for the better handling of image assembly and concurrency.
	The class operates on in-memory arrays via pointer/reference access. It uses a
	linearly increasing pulseID index to provide some headroom for collecting frames
	from multiple detectors.
	**/
class FrameCache{
public:
    FrameCache(uint64_t capacity, uint64_t line_size, uint64_t block_size, std::function<void(ImageMetadata*, std::vector<char>*)> callback):
            m_capacity(capacity), m_linesize(line_size), m_blocksize(block_size), f_send(callback),
            m_vlock(capacity), m_valid(capacity), m_fill(capacity), m_meta(capacity), m_data(capacity) {
        // Reserve the data buffer
        for(auto& it: m_data){ it.resize(m_linesize*m_blocksize); }
    };


    /** Emplace a specific frame and module **/
    void emplace(uint64_t pulseID, uint32_t moduleID, char* ptr_source, ModuleFrame* ptr_meta){
        uint64_t idx = pulseID % m_capacity;

        // Wait for unlocking block
        while(m_vlock[idx]){ std::this_thread::yield(); }

        // Invalid cache line: Just start a new line
        if(m_valid[idx]){ start_line(idx, ptr_meta); }

        // A new frame is starting
        if(ptr_meta->frame_index != m_meta[idx].frame_index){
            flush_line(idx);
            start_line(idx, ptr_meta);
        }

        m_fill[idx]++;
        char* ptr_dest = m_data[idx].data() + moduleID * m_blocksize;
        memcpy(ptr_dest, (void*)ptr_source, m_blocksize);
        memcpy(&m_meta[idx], (void*)ptr_meta, sizeof(ModuleFrame));
    }


    void flush_all(){
        for(int64_t idx=0; idx< m_capacity; idx++){
            flush_line(idx);
        }
    }

    void flush_line(uint64_t idx){
        if(m_valid[idx]){
            m_vlock[idx] = 1;
            std::cout << "Send action" << std::endl;
            f_send(&m_meta[idx], &m_data[idx]);

            m_valid[idx] = 0;
            m_fill[idx] = 0;
            m_vlock[idx] = 0;
        }
    }

    void start_line(uint64_t idx, ModuleFrame* ptr_meta){
        m_vlock[idx] = 1;
        m_meta[idx].pulse_id = ptr_meta->pulse_id;
        m_meta[idx].frame_index = ptr_meta->frame_index;
        m_meta[idx].daq_rec = ptr_meta->daq_rec;
        m_meta[idx].is_good_image = true;
        m_valid[idx] = 1;
        m_fill[idx] = 0;
        m_vlock[idx] = 0;
    }

private:
    const uint64_t m_capacity;
    const uint64_t m_linesize;
    const uint64_t m_blocksize;
    std::function<void(ImageMetadata*, std::vector<char>*)> f_send;

    /** Main container and mutex guard **/
    std::vector<std::atomic<uint32_t>> m_vlock;
    std::vector<std::atomic<uint32_t>> m_valid;
    std::vector<std::atomic<uint32_t>> m_fill;
    std::vector<ImageMetadata> m_meta;
    std::vector<std::vector<char>> m_data;
};

#endif // FRAME_CACHE_HPP
