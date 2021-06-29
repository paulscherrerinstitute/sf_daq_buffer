#ifndef FRAME_CACHE_HPP
#define FRAME_CACHE_HPP

#include <cstddef>
#include <cstring>
#include <stdexcept>
#include <iostream>
#include <mutex>
#include <shared_mutex>
#include <vector>
#include <atomic>
#include <functional>
#include <thread>
#include <mutex>

#include "../../core-buffer/include/formats.hpp"


/** Frame cache

	Reimplemented RamBuffer for better concurrency.
	The class operates on in-memory arrays via pointer/reference access. It uses a
	linearly increasing pulseID index to provide some headroom for collecting frames
	from multiple detectors.
	**/
class FrameCache{
public:
    FrameCache(uint64_t _C, uint64_t N_MOD, std::function<void(ImageBinaryFormat&)> callback):
            m_CAP(_C), m_M(N_MOD),
            m_buffer(_C, ImageBinaryFormat(512*N_MOD, 1024, sizeof(uint16_t))),
            f_send(callback), m_lock(_C), m_valid(_C) {
        // Initialize buffer metadata
        for(auto& it: m_buffer){ memset(&it.meta, 0, sizeof(it.meta)); }
        
        // Initialize Mutexes
        for(auto& it: m_valid){ it = 0; }
    };


    /** Emplace

    Place a recorded frame to it's corresponding module location.
    This simultaneously handles buffering and assembly. **/
    void emplace(uint64_t pulseID, uint64_t moduleIDX, BufferBinaryFormat& ref_frame){
        uint64_t idx = pulseID % m_CAP;
        
        // A new frame is starting
        if(ref_frame.meta.pulse_id != m_buffer[idx].meta.pulse_id){
            start_line(idx, ref_frame.meta);
        }        
        
        // Shared lock for concurrent PUT operations
        std::shared_lock<std::shared_mutex> guard(m_lock[idx]);

        // Copy metadata (manually for now...)
        m_buffer[idx].meta.pulse_id = ref_frame.meta.pulse_id;
        m_buffer[idx].meta.frame_index = ref_frame.meta.frame_index;
        m_buffer[idx].meta.daq_rec = ref_frame.meta.daq_rec;
        
        // Calculate destination pointer (easier to debug)
        char* ptr_dest = m_buffer[idx].data.data() + moduleIDX * m_blocksize;
        std::memcpy((void*)ptr_dest, (void*)&ref_frame.data, m_blocksize);
    }

    void flush_all(){
        for(int64_t idx=0; idx< m_CAP; idx++){
            flush_line(idx);
        }
    }

    // Flush and invalidate a line (incl. lock)
    void flush_line(uint64_t idx){
        std::unique_lock<std::shared_mutex> guard(m_lock[idx]);
        if(m_valid[idx]){
            f_send(m_buffer[idx]);
            m_valid[idx] = 0;
        }
    }

    // Flush and start a new line (incl. lock)
    void start_line(uint64_t idx, ModuleFrame& ref_meta){
        // 0. Guard
        std::unique_lock<std::shared_mutex> guard(m_lock[idx]);
        // 1. Flush
        if(m_valid[idx]){
             f_send(m_buffer[idx]);
        }
        // 2. Init
        m_buffer[idx].meta.pulse_id = ref_meta.pulse_id;
        m_buffer[idx].meta.frame_index = ref_meta.frame_index;
        m_buffer[idx].meta.daq_rec = ref_meta.daq_rec;
        m_buffer[idx].meta.is_good_image = true;
        m_valid[idx] = 1;
    }

private:
    const uint64_t m_CAP;
    const uint64_t m_M;
    const uint64_t m_blocksize = 1024*512*sizeof(uint16_t);
    std::function<void(ImageBinaryFormat&)> f_send;

    /** Main container and mutex guard **/
    std::vector<std::shared_mutex> m_lock;
    std::vector<std::atomic<uint32_t>> m_valid;
    std::vector<ImageBinaryFormat> m_buffer;
};

#endif // FRAME_CACHE_HPP
