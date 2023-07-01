#pragma once

#include <chrono>

namespace Sol {

typedef std::chrono::steady_clock Time;
typedef std::chrono::milliseconds ms;
typedef std::chrono::duration<float> fsec;
typedef std::chrono::steady_clock::time_point TimePoint;

struct Clock {
    static Clock *instance();

    TimePoint now = Time::now();

    // returns the time difference from when it was last called
    float set_time() {
        auto t = Time::now();
        auto fs = t - now;
        now = t;
        return fs.count();
    }
    // returns the time until dur == 0
    float timer(fsec dur);
};


} // namespace Sol
