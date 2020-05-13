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

namespace foolysh {
namespace tools {

/**
 * Set the callback function from Python
 */
void TaskManager::
set_callback(callback cb) {
    _cb = cb;
}

/**
 * Add a new task with unique ``name``.
 */
void TaskManager::
add_task(std::string name, const double delay, const bool with_dt,
         void* pyobj) {
    Task t;
    t.pyobj = pyobj;
    t.delay = delay;
    t.remaining = delay;
    t.with_dt = with_dt;
    _tasks.emplace(std::make_pair(name, t));
}

/**
 * Remove task ``name``.
 */
void TaskManager::
remove_task(std::string name) {
    _delete_list.emplace_back(name);
}

/**
 * To be called every frame.
 */
void TaskManager::
execute(const double dt) {
    for (auto name : _delete_list) {
        _tasks.erase(name);
    }
    _delete_list.clear();
    // _clock->tick();
    // const double dt = _clock->get_dt();
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
            _cb(t.pyobj, it->first, _dt, t.with_dt);
            t.remaining = t.delay;
        }
    }
}

/**
 * Change delay for task ``name``.
 */
void TaskManager::
set_delay(std::string name, const double delay) {
    Task& t = _tasks[name];
    t.delay = t.remaining = delay;
}

/**
 * Pause task ``name``.
 */
void TaskManager::
pause(std::string name) {
    Task& t = _tasks[name];
    t.running = false;
}

/**
 * Resume task ``name``.
 */
void TaskManager::
resume(std::string name) {
    Task& t = _tasks[name];
    t.running = true;
}

/**
 * Return ``true`` if task ``name`` is running, otherwise ``false``.
 */
bool TaskManager::
state(std::string name) {
    Task& t = _tasks[name];
    return t.running;
}

/**
 * Return the delay for task ``name``.
 */
double TaskManager::
get_delay(std::string name) {
    Task& t = _tasks[name];
    return t.delay;
}


}  // namespace tools
}  // namespace foolysh
