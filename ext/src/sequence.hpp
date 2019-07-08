#ifndef SEQUENCE_HPP
#define SEQUENCE_HPP

#include <vector>
#include <initializer_list>

namespace animation {
    class Interval;

    class Sequence {
    public:
        Sequence();
        Sequence(std::initializer_list<Interval> intervals);
        void add_interval(const Interval& interval);
        void play();
        void loop(bool on = true);
        void pause();
        void stop();
        void reset();
        bool playing();
        bool looping();
        void execute_step(const double dt);

    private:
        std::vector<Interval> _intervals;
        bool _playing, _looping;
        std::vector<Interval>::size_type _current_interval_id;
    };
}  // namespace animation

#endif
