#ifndef FRAME_CACHE_HPP
#define FRAME_CACHE_HPP

#include <cstddef>
#include <cstring>
#include <stdexcept>
#include <iostream>
#include <mutex>
#include <vector>
#include <atomic>
#include <functional>
#include <thread>

#include "../../core-buffer/include/formats.hpp"


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
    void emplace(uint64_t pulseID, uint64_t moduleID, BufferBinaryFormat& ref_frame){
        uint64_t idx = pulseID % m_capacity;
        std::cout << "  Emplace " << idx  << "( " <<  ref_frame.meta.frame_index << " to " << m_meta[idx].frame_index << " )" << std::endl;
        
        // Wait for unlocking block
        while(m_vlock[idx]){ std::this_thread::yield(); }

        // Invalid cache line: Just start a new line
        if(m_valid[idx]){ start_line(idx, ref_frame.meta); }

        // A new frame is starting
        if(ref_frame.meta.frame_index != m_meta[idx].frame_index){
            std::cout << "  New frame " << std::endl;
            flush_line(idx);
            start_line(idx, ref_frame.meta);
        }

        std::cout << "    fill/cpy" << std::endl;
        m_fill[idx]++;
        char* ptr_dest = m_data[idx].data() + moduleID * m_blocksize;
        std::memcpy(ptr_dest, (void*)&ref_frame.data, m_blocksize);
        std::memcpy(&m_meta[idx], (void*)&ref_frame.meta, sizeof(ModuleFrame));
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

    void start_line(uint64_t idx, ModuleFrame& ref_meta){
        m_vlock[idx] = 1;
        m_meta[idx].pulse_id = ref_meta.pulse_id;
        m_meta[idx].frame_index = ref_meta.frame_index;
        m_meta[idx].daq_rec = ref_meta.daq_rec;
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
