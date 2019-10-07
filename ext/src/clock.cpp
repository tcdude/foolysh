/**
 * Copyright (c) 2019 Tiziano Bettio
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */


#include "clock.hpp"
#include <chrono>

void tools::Clock::
tick() {
    using namespace std::chrono;
    long current = duration_cast<nanoseconds>(
        high_resolution_clock::now().time_since_epoch()).count();

    if (_clock_stat.start > 0) {
        _clock_stat.delta_time = (double)(current - _clock_stat.current) * 1e-9;
        _clock_stat.current = current;
    }
    else {
        _clock_stat.start = _clock_stat.current = current;
        _clock_stat.delta_time = 0.0;
    }
}

double tools::Clock::
get_dt() {
    if (_clock_stat.start < 0) {
        tick();
    }
    return _clock_stat.delta_time;
}

double tools::Clock::
get_time() {
    if (_clock_stat.start < 0) {
        tick();
    }
    return (double)(_clock_stat.current - _clock_stat.start) * 1e-9;
}

