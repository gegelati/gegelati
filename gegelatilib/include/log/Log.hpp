#ifndef __LOG_HPP__
#define __LOG_HPP__

#include <iostream>


class Logger {
protected:
    std::ostream *out;

public:
    Logger() : out(&std::cout) {};

    Logger(std::ostream &out) : out(&out) {};

    void operator<<(std::ostream &(*manip)(std::ostream &));

    template<typename T>
    void operator<<(const T &val) {
        *out << val;
    }
};


#endif  // __LOG_HPP__
