#include <log/LALogger.h>

double LALogger::getDurationFrom(std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>& time) {
    return (getTime() - time).count();
}

std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> LALogger::getTime() {
    return std::chrono::system_clock::now();
}

void LALogger::chronoFromNow() {
    auto timeNow = getTime();
    start = &timeNow;
}
