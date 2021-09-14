#ifndef FRAME_CACHE_HPP
#define FRAME_CACHE_HPP

#include <thread>
#include <atomic>
#include <chrono>
#include <mutex>
#include <iostream>


/** Watchdog timer class

    Unless kicked repeatedly, it periodically calls a user-defined function.
    **/
class Watchdog{
public:
    Watchdog(int64_t timeout, std::function<void()> callback): m_timeout(timeout), m_callback(callback) {};
    ~Watchdog() { Stop(); };
    void Start();
    void Stop();
    void Kick();

protected:
    int64_t m_timeout;
    std::atomic<bool> m_running = false;
    std::function<void()> m_callback;
    std::chrono::time_point<std::chrono::steady_clock> m_lastkick;

    std::thread m_thread;
    std::mutex m_mutex;
    void Loop();
};


void Watchdog::Start(){
    std::unique_lock<std::mutex> lock(m_mutex);
    if(m_running == false){
        m_running = true;
        m_lastkick = std::chrono::steady_clock::now();
        m_thread = std::thread(&Watchdog::Loop, this);
    }
}

void Watchdog::Stop(){
    std::unique_lock<std::mutex> g_guard(m_mutex);
    if(m_running == true){
        m_running = false;
        m_thread.join();
    }
}

void Watchdog::Kick(){
    std::unique_lock<std::mutex> g_guard(m_mutex);
    m_lastkick = std::chrono::steady_clock::now();
}

void Watchdog::Loop(){
    std::cout << "Starting watchdog" << std::endl;
    while(m_running){
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - m_lastkick);
        if(elapsed.count() < m_timeout){
            // std::cout << "Elapsed " << (int64_t)elapsed.count() << " of " << m_timeout << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        } else {
            std::cout << "Expired timer" << std::endl;
            m_callback();
            // Infinite re-kick
            std::unique_lock<std::mutex> g_guard(m_mutex);
            m_lastkick = std::chrono::steady_clock::now();
        }
    }
}

#endif // FRAME_CACHE_HPP
