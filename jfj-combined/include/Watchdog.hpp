#ifndef FRAME_CACHE_HPP
#define FRAME_CACHE_HPP

#include <thread>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <iostream>


class Watchdog{
public:
    Watchdog(uint32_t timeout, std::function<void()> callback): m_timeout(timeout), m_callback(callback) {
        m_timeout = timeout;
        m_callback = callback;
        m_running = false;
    };
    ~Watchdog() { Stop(); };
    void Start();
    void Stop();
    void Kick();

private:
    std::atomic<bool> m_running = false;
    std::function<void()> m_callback;
    uint32_t m_timeout;
    std::chrono::time_point m_lastKick;


    std::thread m_thread;
    std::mutex m_mutex;
    steady_clock::time_point m_lastPetTime;
    std::condition_variable m_stopCondition;
    void Loop();
};


void Watchdog::Start(){
    std::unique_lock<std::mutex> lock(m_mutex);
    if(m_running == false){
        m_running = true;
        m_lastKick = std::chrono::steady_clock::now();
        m_thread = std::thread(&Watchdog::Loop, this);
    }
}

void Watchdog::Stop(){
    std::unique_lock<std::mutex> locker(m_mutex);
    if(m_running == true){
        m_running = false;
        m_thread.join();
    }
}

void Watchdog::Kick(){
    std::unique_lock<std::mutex> locker(m_mutex);
    m_lastKick = steady_clock::now();
}

void Watchdog::Loop(){
    while(m_running){
        if((std::chrono::now() - m_last_kick) < m_timeout){
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        } else {
            std::cout << "Expired timer" << std::endl;
            m_callback();
        }
    }
}

#endif // FRAME_CACHE_HPP
