#include "log/Log.hpp"

#include <iostream>

void Logger::operator<<(std::ostream &(*manip)(std::ostream &)) {
    manip(*out);
}




