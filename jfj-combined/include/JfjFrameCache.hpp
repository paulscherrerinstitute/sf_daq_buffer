#ifndef SF_DAQ_FRAME_CACHE_HPP
#define SF_DAQ_FRAME_CACHE_HPP

#include <iostream>
#include <cstring>
#include <vector>
#include <functional>
#include <shared_mutex>

#include "../../core-buffer/include/formats.hpp"
#include "Watchdog.hpp"


/** Frame cache

	Reimplemented threadsafe RamBuffer that handles concurrency internally via mutexes.
	The class operates on in-memory arrays via pointer/reference access. It uses a
	linearly increasing pulseID index for cache addressing. The standard placement method
	ensures that no data corruption occurs, lines are always flushed before overwrite.
	A large-enough buffer should ensure that there is sufficient time to retrieve all
	data from all detector modules.

	TODO: The class is header-only for future template-refactoring.
	**/
class FrameCache{
public:
    FrameCache(uint64_t _C, uint64_t N_MOD, std::function<void(ImageBinaryFormat&)> callback):
            m_CAP(_C), m_M(N_MOD), m_valid(_C, 0), m_lock(_C),
            m_buffer(_C, ImageBinaryFormat(512*N_MOD, 1024, sizeof(uint16_t))),
            f_send(callback), m_watchdog(500, flush_all) {
        // Initialize buffer metadata
        for(auto& it: m_buffer){ memset(&it.meta, 0, sizeof(it.meta)); }
        m_watchdog.Start();
    };


    /** Emplace

    Place a recorded frame to it's corresponding module location.
    This simultaneously handles buffering, assembly and flushing.
    Also handles concurrency (shared and unique mutexes).

    NOTE: Forced flushing is performed by the current thread.
    **/
    void emplace(uint64_t pulseID, uint64_t moduleIDX, BufferBinaryFormat& inc_frame){
        // Cache-line index
        const uint64_t idx = pulseID % m_CAP;

        // A new frame is starting
        if(inc_frame.meta.pulse_id != m_buffer[idx].meta.pulse_id){
            // Unique lock to flush and start a new one
            std::unique_lock<std::shared_mutex> p_guard(m_lock[idx]);
            // Check if condition persists after getting the mutex
            if(inc_frame.meta.pulse_id != m_buffer[idx].meta.pulse_id){
                start_line(idx, inc_frame.meta);
            }
        }

        // Shared lock for concurrent PUT operations
        std::shared_lock<std::shared_mutex> s_guard(m_lock[idx]);

        // Calculate destination pointer and copy data
        char* ptr_dest = m_buffer[idx].data.data() + moduleIDX * m_blocksize;
        std::memcpy((void*)ptr_dest, (void*)&inc_frame.data, m_blocksize);
    }

    void flush_all(){
        for(int64_t idx=0; idx< m_CAP; idx++){
            std::unique_lock<std::shared_mutex> p_guard(m_lock[idx]);
            flush_line(idx);
        }
    }

    /** Flush and invalidate a line

    Flushes a valid cache line and invalidates the associated buffer.
    NOTE : It does not lock, that must be done externally!        **/
    void flush_line(uint64_t idx){
        if(m_valid[idx]){
            f_send(m_buffer[idx]);
            m_valid[idx] = 0;
        }
    }

    /** Flush and start a new line

    Flushes a valid cache line and starts another one from the provided metadata.
    NOTE : It does not lock, that must be done externally!        **/
    void start_line(uint64_t idx, ModuleFrame& inc_frame){
        // 1. Flush
        if(m_valid[idx]){ f_send(m_buffer[idx]); }

        // 2. Init new frame
        m_buffer[idx].meta.pulse_id = inc_frame.pulse_id;
        m_buffer[idx].meta.frame_index = inc_frame.frame_index;
        m_buffer[idx].meta.daq_rec = inc_frame.daq_rec;
        m_buffer[idx].meta.is_good_image = true;
        m_valid[idx] = 1;
    }

private:
    const uint64_t m_CAP;
    const uint64_t m_M;
    const uint64_t m_blocksize = 1024*512*sizeof(uint16_t);

    /** Flush function **/
    std::function<void(ImageBinaryFormat&)> f_send;

    /** Main container and mutex guard **/
    std::vector<uint32_t> m_valid;
    std::vector<std::shared_mutex> m_lock;
    std::vector<ImageBinaryFormat> m_buffer;

    /** Watchdog timer **/
    Watchdog m_watchdog;
};

#endif // SF_DAQ_FRAME_CACHE_HPP
