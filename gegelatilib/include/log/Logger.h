#ifndef GEGELATI_LOGGER_H
#define GEGELATI_LOGGER_H

#include <iostream>

/// Logger class enabling to log elements in a given output stream
class Logger {
protected:
    /// output stream where all what is logged is brought
    std::ostream *out;

public:
    /// basic constructor printing on cout
    Logger() : out(&std::cout) {};

    /// constructor initializing a specific output
    Logger(std::ostream &out) : out(&out) {};

    /// << operator to manipulate stream
    Logger operator<<(std::ostream &(*manip)(std::ostream &));

    /// << operator allowing to log elements that ostream actually accepts (char, int...)
    template<typename T>
    Logger operator<<(const T &val) {
        *out << val;
        return *this;
    }
};


#endif
