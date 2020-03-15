/**
 * Copyright (c) 2020 Tiziano Bettio
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

namespace foolysh {
namespace tools {

void Clock::
tick() {
    using std::chrono::duration;
    using std::chrono::time_point;
    auto current = _clock.now();

    if (_clock_stat.init) {
        _clock_stat.delta_time = current - _clock_stat.current;
        _clock_stat.current = current;
    }
    else {
        _clock_stat.start = current;
        _clock_stat.current = current;
        _clock_stat.delta_time = std::chrono::duration<double>::zero();
    }
}

double Clock::
get_dt() {
    if (!_clock_stat.init) {
        tick();
    }
    return _clock_stat.delta_time.count();
}

double Clock::
get_time() {
    using std::chrono::duration;
    if (!_clock_stat.init) {
        tick();
    }
    duration<double> total = _clock_stat.current - _clock_stat.start;
    return total.count();
}


}  // namespace tools
}  // namespace foolysh
