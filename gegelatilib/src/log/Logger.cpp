#include "log/Logger.h"

#include <iostream>

Log::Logger Log::Logger::operator<<(std::ostream &(*manip)(std::ostream &)) {
    manip(*out);
    return *this;
}

