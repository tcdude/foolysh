#ifndef TASKMGR_HPP
#define TASKMGR_HPP

#include <map>
#include <string>
#include <memory>

namespace tools {

    struct Task {
        void* func;
        void* args;
        void* kwargs;
        bool with_dt, running;
        double delay, remaining;
    };

    class Clock;

    typedef void (*callback)(void* func, void* args, void* kwargs,
                             const double dt, const bool with_dt);

    class TaskManager {
    public:
        TaskManager();
        void set_callback(callback cb);
        void add_task(std::string name, const double delay, const bool with_dt,
                      void* func, void* args, void* kwargs);
        void remove_task(std::string);
        void execute();

    private:
        std::map<std::string, Task> _tasks;
        callback _cb;
        std::unique_ptr<Clock> _clock;
    };

}  // namespace tools

#endif
