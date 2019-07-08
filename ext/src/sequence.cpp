#include "sequence.hpp"

#include "interval.hpp"

/**
 * Initialize empty Sequence
 */
animation::Sequence::
Sequence() {
    _playing = _looping = false;
    _current_interval_id = 0;
}

/**
 * Initialize Sequence with Intervals.
 */
animation::Sequence::
Sequence(std::initializer_list<Interval> intervals) : _intervals(intervals) {
    _playing = _looping = false;
    _current_interval_id = 0;
}

/**
 * Add an Interval to the end of this sequence.
 */
void animation::Sequence::
add_interval(const Interval& interval) {
    _intervals.push_back(interval);
}

/**
 * Start Sequence.
 */
void animation::Sequence::
play() {
    _playing = true;
}

/**
 * Set loop flag (default=true).
 */
void animation::Sequence::
loop(bool on) {
    _looping = on;
}

/**
 * Pause Sequence.
 */
void animation::Sequence::
pause() {
    _playing = false;
}

/**
 * Stop Sequence
 */
void animation::Sequence::
stop() {
    _playing = false;
    _current_interval_id = 0;
}

/**
 *
 */
void animation::Sequence::
reset() {
    if (_current_interval_id < _intervals.size()) {
        _intervals[_current_interval_id].reset();
    }
    _current_interval_id = 0;
    _intervals[_current_interval_id].reset();
}

/**
 *
 */
bool animation::Sequence::
playing() {
    return _playing;
}

/**
 *
 */
bool animation::Sequence::
looping() {
    return _looping;
}

/**
 *
 */
void animation::Sequence::
execute_step(const double dt) {
    if (_playing) {
        double rem = -1.0, ddt = dt;
        while (rem <= 0) {
            rem = _intervals[_current_interval_id].execute_step(ddt);
            if (rem <= 0) {
                ddt = -rem;
                ++_current_interval_id;
                if (_current_interval_id == _intervals.size()) {
                    if (_looping) {
                        _current_interval_id = 0;
                    }
                    else {
                        _playing = false;
                        reset();
                        break;
                    }
                }
                _intervals[_current_interval_id].reset();
            }
        }
    }
}
