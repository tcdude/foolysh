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
