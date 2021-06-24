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

	Reimplemented RamBuffer for better concurrency.
	The class operates on in-memory arrays via pointer/reference access. It uses a
	linearly increasing pulseID index to provide some headroom for collecting frames
	from multiple detectors.
	**/
class FrameCache{
public:
    FrameCache(uint64_t _C, uint64_t _MX, uint64_t _MY, std::function<void(ImageBinaryFormat&)> callback):
            m_CAP(_C), m_MX(_MX), m_MY(_MY), m_M(_MX*_MY), m_PX(1024*_MX), m_PY(512*_MY),
            m_buffer(_C, ImageBinaryFormat(512*_MY, 1024*_MX, sizeof(uint16_t))),
            f_send(callback), m_vlock(_C), m_valid(_C), m_fill(_C) {
    };


    /** Emplace

    Place a recorded frame to it's corresponding module location.
    This simultaneously handles buffering and assembly. **/
    void emplace(uint64_t pulseID, uint64_t moduleIDX, BufferBinaryFormat& ref_frame){
        uint64_t idx = pulseID % m_CAP;
        std::cout << "  Emplace: " << idx << std::endl;


        // Wait for unlocking block
        // while(m_vlock[idx]){ std::this_thread::yield(); }

        // Invalid cache line: Just start a new line
        //if(m_valid[idx]){ start_line(idx, ref_frame.meta); }

        // A new frame is starting
        std::cout << "  Pulse_ids: " << ref_frame.meta.pulse_id << "\t" << m_buffer[idx].meta.pulse_id << std::endl;

        if(ref_frame.meta.pulse_id != m_buffer[idx].meta.pulse_id){
            std::cout << "NOT EQUAL" << std::endl;
            flush_line(idx);
            start_line(idx, ref_frame.meta);
        }

        std::cout << "    fill/cpy" << std::endl;
        m_fill[idx]++;
        char* ptr_dest = m_buffer[idx].data + moduleIDX * m_blocksize;
                std::cout << "    target:" << (void*)ptr_dest << "\tsize: " << m_blocksize << std::endl;

        m_buffer[idx].meta.pulse_id = ref_frame.meta.pulse_id;
        m_buffer[idx].meta.frame_index = ref_frame.meta.frame_index;
        m_buffer[idx].meta.daq_rec = ref_frame.meta.daq_rec;
        std::cout << "NI " << std::endl;
        std::memcpy((void*)ptr_dest, (void*)&ref_frame.data, m_blocksize);
        
        std::cout << "    Fill ctr: " <<  m_fill[idx]  << std::endl;

    }

    void flush_all(){
        for(int64_t idx=0; idx< m_CAP; idx++){
            flush_line(idx);
        }
    }

    void flush_line(uint64_t idx){
        if(m_valid[idx]){
            std::cout << "Flushing line: " << idx << std::endl;
            m_vlock[idx] = 1;
            f_send(m_buffer[idx]);
            m_valid[idx] = 0;
            m_fill[idx] = 0;
            m_vlock[idx] = 0;
        }
    }

    void start_line(uint64_t idx, ModuleFrame& ref_meta){
        m_vlock[idx] = 1;
        m_buffer[idx].meta.pulse_id = ref_meta.pulse_id;
        m_buffer[idx].meta.frame_index = ref_meta.frame_index;
        m_buffer[idx].meta.daq_rec = ref_meta.daq_rec;
        m_buffer[idx].meta.is_good_image = true;
        m_valid[idx].exchange(1);
        m_fill[idx] = 0;
        m_vlock[idx] = 0;
    }

private:
    const uint64_t m_CAP;
    const uint64_t m_PX;
    const uint64_t m_PY;
    const uint64_t m_MX;
    const uint64_t m_MY;
    const uint64_t m_M;
    const uint64_t m_blocksize = 1024*512*sizeof(uint16_t);
    std::function<void(ImageBinaryFormat&)> f_send;

    /** Main container and mutex guard **/
    std::vector<std::atomic<uint32_t>> m_vlock;
    std::vector<std::atomic<uint32_t>> m_valid;
    std::vector<std::atomic<uint32_t>> m_fill;
    std::vector<ImageBinaryFormat> m_buffer;
};

#endif // FRAME_CACHE_HPP
