#include <log/LALogger.h>

double LALogger::getDurationFrom(std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>& time) {
    return ((std::chrono::duration<double>)(getTime() - time)).count();
}

std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> LALogger::getTime() {
    return std::chrono::system_clock::now();
}

void LALogger::chronoFromNow() {
    checkpoint = std::make_shared<std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>>(getTime());
}
