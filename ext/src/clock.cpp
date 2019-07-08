#include "clock.hpp"
#include <chrono>

void tools::Clock::
tick() {
    using namespace std::chrono;
    long current = duration_cast<nanoseconds>(
        high_resolution_clock::now().time_since_epoch()).count();

    if (_clock_stat.start > 0) {
        _clock_stat.delta_time = static_cast<double>(current - _clock_stat.current) * 1e-9;
        _clock_stat.current = current;
    }
    else {
        _clock_stat.start = _clock_stat.current = current;
        _clock_stat.delta_time = 0.0;
    }
}
