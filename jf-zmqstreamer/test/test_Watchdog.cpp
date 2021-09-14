#include "gtest/gtest.h"
#include "Watchdog.hpp"

#include <thread>
#include <chrono>
#include <future>

using namespace std;


uint64_t tick_counter = 0;
// Dummy callback to increase tick counter
void mock_callback(){
    tick_counter++;
};


TEST(WatchdogTimer, timer_test){
    Watchdog wDog(100, &mock_callback);

    // Free running
    wDog.Start();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    ASSERT_EQ(tick_counter, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    ASSERT_EQ(tick_counter, 5);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    ASSERT_EQ(tick_counter, 10);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    ASSERT_EQ(tick_counter, 15);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    ASSERT_EQ(tick_counter, 20);

    // Test Stop()
    wDog.Stop();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    ASSERT_EQ(tick_counter, 20);

    // Test Kick()
    tick_counter = 0;
    wDog.Start();
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    ASSERT_EQ(tick_counter, 2);
    for(int ii=0; ii<20 ii++){
        wDog.Kick();
        ASSERT_EQ(tick_counter, 2);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    ASSERT_EQ(tick_counter, 4);
}
