#include "time.hpp"
#include <chrono>
#include <string>

long long Time::nanos() {
    auto currentTime = std::chrono::high_resolution_clock::now();
    // Convert the time point to nanoseconds since epoch
    auto nanoseconds_since_epoch = std::chrono::time_point_cast<std::chrono::nanoseconds>(currentTime).time_since_epoch().count();
    // Store the nanoseconds in a long long
    long long currentNanoseconds = nanoseconds_since_epoch;
    return currentNanoseconds;
}