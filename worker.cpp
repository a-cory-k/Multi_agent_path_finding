#include "world.h"
#include <thread>
#include <chrono>

void worker() {
    while (true) {
        if (simulationStarted) {
            updateLogic();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}