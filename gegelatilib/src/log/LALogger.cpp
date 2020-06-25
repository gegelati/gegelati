#include "log/LALogger.h"

double Log::LALogger::getDurationFrom(const std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>& time) const {
    return ((std::chrono::duration<double>)(getTime() - time)).count();
}

std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> Log::LALogger::getTime() const {
    return std::chrono::system_clock::now();
}

void Log::LALogger::chronoFromNow() {
    checkpoint = std::make_shared<std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>>(getTime());
}
