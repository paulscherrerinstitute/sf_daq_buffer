#ifndef SF_DAQ_FRAME_CACHE_HPP
#define SF_DAQ_FRAME_CACHE_HPP

#include <iostream>
#include <cstring>
#include <deque>
#include <thread>
#include <vector>
#include <functional>
#include <shared_mutex>

#include "../../core-buffer/include/EpicsFieldTypes.hpp"
#include "../../core-buffer/include/formats.hpp"
#include "Watchdog.hpp"

#define MAX_FIFO_LENGTH 32


/** Frame Cache

	Similar to a thread-safe RamBuffer that handles concurrency internally via mutexes.
	The class operates on in-memory arrays via pointer/reference access. It uses a
	linearly increasing pulseID index for cache addressing. The standard placement method
	ensures that no data corruption occurs, lines are always flushed before overwrite.
	A large-enough buffer should ensure that there is sufficient time to retrieve all
	data from all detector modules.

	The cache line is flushed on three occasions:
    - A new frame is about to overwrite it (by the frame-worker thread)
    - Complete frames are queued for flushing internally (by internal worker)
    - Incomplete frames are flushed by a watchdog after a timeout (by watchdog worker)

	NOTE: The class is header-only for future template-refactoring.
	TODO: Multiple queue workers
	**/
class FrameCache{
public:
    FrameCache(uint64_t N_CAP, uint64_t modX, uint64_t modY, std::function<void(ImageBinaryFormat&)> callback):
            m_capacity(N_CAP), m_modX(modX), m_modY(modY), m_mod(modX*modY), m_valid(N_CAP, 0), m_fill(N_CAP, 0), m_lock(N_CAP),
            m_buffer(N_CAP, ImageBinaryFormat(512*N_MOD, 1024, sizeof(uint16_t))),
            f_send(callback) {
        // Initialize buffer metadata
        for(auto& it: m_buffer){ memset(&it.meta, 0, sizeof(it.meta)); }

        // Initialize the watchdog
        std::function<void()> wd_callback = std::bind(&FrameCache::flush_all, this);
        m_watchdog = new Watchdog(500, wd_callback);
        m_watchdog->Start();

        // Start drain worker
        m_drainer = std::thread(&FrameCache::drain_loop, this);
    };


    /** Emplace

    Place a recorded frame to it's corresponding module location.
    This simultaneously handles buffering, assembly and flushing.
    Also handles concurrency (shared and unique mutexes).

    NOTE: Forced flushing is performed by the current thread.
    **/
    void emplace(uint64_t pulseID, uint64_t moduleIDX, BufferBinaryFormat& inc_frame){
        // Cache-line index
        const uint64_t idx = pulseID % m_capacity;

        // A new frame is starting
        if(inc_frame.meta.pulse_id != m_buffer[idx].meta.id){
            // Unique lock to flush and start a new one
            std::unique_lock<std::shared_mutex> p_guard(m_lock[idx]);
            // Check if condition persists after getting the mutex
            if(inc_frame.meta.pulse_id != m_buffer[idx].meta.id){
                start_line(idx, inc_frame.meta);
            }
        }

        // Shared lock for concurrent PUT operations
        std::shared_lock<std::shared_mutex> s_guard(m_lock[idx]);

        // Calculate destination pointer and copy data
        char* ptr_dest = m_buffer[idx].data.data() + moduleIDX * m_blocksize;
        std::memcpy((void*)ptr_dest, (void*)&inc_frame.data, m_blocksize);
        m_fill[idx]++;
        m_watchdog->Kick();

        // Queue for draining
        if(m_fill[idx]==m_mod-1){
            if(m_fill.size() > MAX_FIFO_LENGTH) {
                m_drain_queue.push_back(idx);
            }
        }
    }

protected:
    /** Flush and start a new line

    Flushes a valid cache line and starts another one from the provided metadata.
    NOTE : It does not lock, that must be done externally!        **/
    void start_line(uint64_t idx, ModuleFrame& inc_frame){
        // 1. Flush
        if(m_valid[idx]){ f_send(m_buffer[idx]); }

        // 2. Init new frame
        m_buffer[idx].meta.id = inc_frame.pulse_id;
        m_buffer[idx].meta.width = 1024 * m_modX;
        m_buffer[idx].meta.height = 512 * m_modY;
        m_buffer[idx].meta.dtype = (int)DBF_USHORT;
        m_buffer[idx].meta.user_1 = inc_frame.frame_index;
        m_buffer[idx].meta.user_2 = inc_frame.daq_rec;
        m_buffer[idx].meta.status = true;
        m_fill[idx] = 0;
        m_valid[idx] = 1;
    }

    /** Flush and invalidate a line

    Flushes a valid cache line and invalidates the associated buffer.
    NOTE : It does not lock, that must be done externally!        **/
    void flush_line(uint64_t idx){
        if(m_valid[idx]){
            f_send(m_buffer[idx]);
            m_fill[idx] = 0;
            m_valid[idx] = 0;
        }
    }

    /** Flush all lines in the buffer**/
    void flush_all(){
        for(int64_t idx=0; idx< m_capacity; idx++){
            std::unique_lock<std::shared_mutex> p_guard(m_lock[idx]);
            flush_line(idx);
        }
    }

    /** Drain loop

    Flushes queued frames from the cache buffer and invalidates line.
    It also locks the frame for the duration of flushing!        **/
    void drain_loop(){
        while(true){
            if(!m_drain_queue.empty()){
                uint32_t idx = m_drain_queue.front();
                m_drain_queue.pop_front();
                // Lock and flush the frame
                std::unique_lock<std::shared_mutex> p_guard(m_lock[idx]);
                flush_line(idx);
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
            }
        }
    }

    /** Variables **/
    const uint64_t m_capacity;
    const uint64_t m_modX;
    const uint64_t m_modY;
    const uint64_t m_mod;
    const uint64_t m_blocksize = 1024*512*sizeof(uint16_t);

    /** Flush function **/
    std::function<void(ImageBinaryFormat&)> f_send;

    /** Main container and mutex guard **/
    std::vector<uint32_t> m_valid;
    std::vector<uint32_t> m_fill;
    std::vector<std::shared_mutex> m_lock;
    std::vector<ImageBinaryFormat> m_buffer;

    /** Watchdog timer and flush queue **/
    Watchdog *m_watchdog;
     std::thread m_drainer;
    std::deque<uint32_t> m_drain_queue;

};

#endif // SF_DAQ_FRAME_CACHE_HPP
