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

#include "taskmgr.hpp"
#include "clock.hpp"

/**
 */
tools::TaskManager::
TaskManager() {
    _clock.reset(new Clock());
}

/**
 * Set the callback function from Python
 */
void tools::TaskManager::
set_callback(callback cb) {
    _cb = cb;
}

/**
 * Add a new task with unique ``name``.
 */
void tools::TaskManager::
add_task(std::string name, const double delay, const bool with_dt, void* func,
         void* args, void* kwargs) {
    Task t;
    t.func = func;
    t.args = args;
    t.kwargs = kwargs;
    t.delay = delay;
    t.remaining = delay;
    t.with_dt = with_dt;
    _tasks.emplace(std::make_pair(name, t));
}

/**
 * Remove task ``name``.
 */
void tools::TaskManager::
remove_task(std::string name) {
    _tasks.erase(name);
}

/**
 * To be called every frame.
 */
void tools::TaskManager::
execute() {
    _clock->tick();
    const double dt = _clock->get_dt();
    for (auto it = _tasks.begin(); it != _tasks.end(); ++it) {
        auto& t = _tasks[it->first];
        if (!t.running) {
            continue;
        }
        if (t.delay > 0.0) {
            t.remaining -= dt;
        }
        if (t.remaining <= 0.0) {
            const double _dt = (t.delay > 0) ? t.delay - t.remaining : dt;
            _cb(t.func, t.args, t.kwargs, _dt, t.with_dt);
            t.remaining = t.delay;
        }
    }
}

/**
 * Change delay for task ``name``.
 */
void tools::TaskManager::
set_delay(std::string name, const double delay) {
    Task& t = _tasks[name];
    t.delay = t.remaining = delay;
}

/**
 * Pause task ``name``.
 */
void tools::TaskManager::
pause(std::string name) {
    Task& t = _tasks[name];
    t.running = false;
}

/**
 * Resume task ``name``.
 */
void tools::TaskManager::
resume(std::string name) {
    Task& t = _tasks[name];
    t.running = true;
}

/**
 * Return ``true`` if task ``name`` is running, otherwise ``false``.
 */
bool tools::TaskManager::
state(std::string name) {
    Task& t = _tasks[name];
    return t.running;
}

/**
 * Return the delay for task ``name``.
 */
double tools::TaskManager::
get_delay(std::string name) {
    Task& t = _tasks[name];
    return t.delay;
}
