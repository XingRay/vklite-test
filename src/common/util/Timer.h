//
// Created by leixing on 2025/5/26.
//

#pragma once

#include <chrono>
#include <cstdint>

namespace util {

    class Timer {
    private:
        std::chrono::steady_clock::time_point mStartTime;
        std::chrono::steady_clock::time_point mLastTime;

    public:
        Timer();

        ~Timer();

        void start();

        uint64_t getElapsedTimeMs();

        float getElapsedTimeSecond();

        uint64_t getDeltaTimeMs();

        float getDeltaTimeSecond();
    };

} // util
