#ifndef TIME_HPP
#define TIME_HPP
#include <string>


/**
 * @class Time
 * @brief CLass used for retreiving times as long long.
 */
class Time {
    public:
        /**
         * @return The current nanoseconds.
         */
        static long long nanos();
        /**
         * @return The current seconds.
         */
        static long long seconds();
};

#endif  // TIME_HPP