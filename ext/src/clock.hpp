#ifndef CLOCK_HPP
#define CLOCK_HPP


namespace tools {
    struct {
        long start = -1;
        long current;
        double delta_time;
    } _clock_stat;

    class Clock {
    public:
        void tick();
        inline double get_dt() {
            if (_clock_stat.start < 0) {
                tick();
                return 0.0;
            }
            return _clock_stat.delta_time;
        }
        inline double get_time() {
            if (_clock_stat.start < 0) {
                tick();
                return 0.0;
            }
            return static_cast<double>(_clock_stat.current - _clock_stat.start) * 1e-9;
        }
    };

}  // namespace tools

#endif
