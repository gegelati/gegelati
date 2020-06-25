#include <iostream>

#include "log/Logger.h"

Log::Logger Log::Logger::operator<<(std::ostream &(*manip)(std::ostream &)) {
    manip(*out);
    return *this;
}

