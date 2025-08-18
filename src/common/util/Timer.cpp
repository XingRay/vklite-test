//
// Created by leixing on 2025/5/26.
//

#include "Timer.h"

namespace util {

    Timer::Timer() = default;

    Timer::~Timer() = default;

    void Timer::start() {
        mStartTime = std::chrono::steady_clock::now();
        mLastTime = mStartTime;
    }

    uint64_t Timer::getElapsedTimeMs() {
        auto currentTime = std::chrono::steady_clock::now();
        auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - mStartTime).count();
        return elapsedTime;
    }

    float Timer::getElapsedTimeSecond() {
        return (float) (((double) (getElapsedTimeMs())) / 1000.0);
    }

    uint64_t Timer::getDeltaTimeMs() {
        auto currentTime = std::chrono::steady_clock::now();
        auto deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - mLastTime).count();
        mLastTime = currentTime;
        return deltaTime;
    }

    float Timer::getDeltaTimeSecond() {
        return (float) (((double) (getDeltaTimeMs())) / 1000.0);
    }

} // util