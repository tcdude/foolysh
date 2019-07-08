#include "taskmgr.hpp"
#include "clock.hpp"

tools::TaskManager::
TaskManager() {
    _clock.reset(new Clock());
}

void tools::TaskManager::
set_callback(callback cb) {
    _cb = cb;
}

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

void tools::TaskManager::
execute() {
    _clock->tick();
    const double dt = _clock->get_dt();
    for (auto it : _tasks) {
        Task& t = it.second;
        if (t.delay > 0.0) {
            t.remaining -= dt;
        }
        if (t.remaining <= 0.0) {
            const double _dt = (t.delay > 0) ? t.delay - t.remaining : dt;
            _cb(t.func, t.args, t.kwargs, _dt, t.with_dt);
            t.remaining = t.delay + t.remaining;
        }
    }
}
