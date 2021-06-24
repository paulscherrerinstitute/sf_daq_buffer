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
    FrameCache(uint64_t _C, uint64_t _MX, uint64_t _MY, uint64_t _D, std::function<void(ImageBinaryFormat&)> callback):
            m_CAP(_C), m_MX(_MX), m_MY(_MY), m_M(_MX*_MY), m_PX(1024*_MX), m_PY(512*_MY), m_D(_D),
            m_buffer(_C, ImageBinaryFormat(512*_MY, 1024*_MX, 2)),
            f_send(callback), m_vlock(_C), m_valid(_C), m_fill(_C, 0) {
    };


    /** Emplace

    Place a recorded frame to it's corresponding module location.
    This simultaneously handles buffering and assembly. **/
    void emplace(uint64_t pulseID, uint64_t moduleIDX, BufferBinaryFormat& ref_frame){
        uint64_t idx = pulseID % m_CAP;

        // Wait for unlocking block
        while(m_vlock[idx]){ std::this_thread::yield(); }

        // Invalid cache line: Just start a new line
        if(m_valid[idx]){ start_line(idx, ref_frame.meta); }

        // A new frame is starting
        if(ref_frame.meta.frame_index != m_buffer[idx].meta.frame_index){
            flush_line(idx);
            start_line(idx, ref_frame.meta);
        }

        m_fill[idx]++;
        char* ptr_dest = m_buffer[idx].data() + moduleIDX * m_blocksize;
        std::memcpy(ptr_dest, (void*)&ref_frame.data, m_blocksize);
        std::memcpy(&m_buffer[idx].meta, (void*)&ref_frame.meta, sizeof(ModuleFrame));
    }

    void flush_all(){
        for(int64_t idx=0; idx< m_CAP; idx++){
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
    const uint64_t m_CAP;
    const uint64_t m_MX;
    const uint64_t m_MY;
    const uint64_t m_M;
    const uint64_t m_H;
    const uint64_t m_W;
    const uint64_t m_D;
    std::function<void(ImageMetadata*, std::vector<char>*)> f_send;

    /** Main container and mutex guard **/
    std::vector<std::atomic<uint32_t>> m_vlock;
    std::vector<std::atomic<uint32_t>> m_valid;
    std::vector<std::atomic<uint32_t>> m_fill;
    std::vector<ImageBinaryFormat> m_buffer;
};

#endif // FRAME_CACHE_HPP
