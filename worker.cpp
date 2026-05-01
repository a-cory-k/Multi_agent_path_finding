#include "world.h"
#include <thread>
#include <atomic>

std::atomic<bool> running = true;

void worker() {
    while (running) {
        updateLogic();
        std::this_thread::sleep_for(std::chrono::milliseconds(400));
    }
}