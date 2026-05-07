#pragma once
#include "../VESState.h"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace utils {
    // Returns TRUE only if the time string changed since the last check!
    inline bool tick() {
        static std::string lastTime = ""; // Remembers the state between loops!

        auto now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&now_c), "%H:%M:%S");
        
        std::string currentTime = ss.str();

        // Did the clock tick over to a new second?
        if (currentTime != lastTime) {
            lastTime = currentTime;
            VESState::values["utils.Time"] = currentTime;
            return true; // Yes! Data changed!
        }
        
        return false; // No change. Keep the CPU asleep.
    }
}