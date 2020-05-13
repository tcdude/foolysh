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


#ifndef TASKMGR_HPP
#define TASKMGR_HPP

#include <map>
#include <vector>
#include <string>
#include <memory>

namespace foolysh {
namespace tools {

    struct Task {
        void* pyobj = nullptr;
        bool with_dt, running = true;
        double delay, remaining;
    };

    typedef void (*callback)(void* pyobj, const std::string taskname,
                             const double dt, const bool with_dt);

    class TaskManager {
    public:
        TaskManager() {};
        void set_callback(callback cb);
        void add_task(std::string name, const double delay, const bool with_dt,
                      void* pyobj);
        void remove_task(std::string);
        void execute(const double dt);
        void set_delay(std::string name, const double delay);
        void pause(std::string name);
        void resume(std::string name);
        bool state(std::string name);
        double get_delay(std::string name);

    private:
        std::map<std::string, Task> _tasks;
        std::vector<std::string> _delete_list;
        callback _cb;
    };

}  // namespace tools
}  // namespace foolysh

#endif
