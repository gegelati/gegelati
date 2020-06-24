#ifndef GEGELATI_LOGGER_H
#define GEGELATI_LOGGER_H

#include <iostream>

namespace Log {

/**
* Logger class enabling to log elements in a given output stream.
*/
    class Logger {
    protected:
        /**
        * output stream where all what is logged is put.
        */
        std::ostream *out;

    public:
        /**
        * \brief basic constructor logging on cout.
        */
        Logger() : out(&std::cout) {};

        /**
        * \brief constructor initializing a specific output.
        * \param[in] out the output stream we want to log things to as ostream.
        */
        Logger(std::ostream &out) : out(&out) {};

        /**
        * \brief << operator to manipulate stream and enter stream-specific
        * elements (like std::endl).
        * \param[in] manip the element we want to add.
        * \return the samme logger to be able to stream several things
        * (e.g. logger<<elA<<elB).
        */
        Logger operator<<(std::ostream &(*manip)(std::ostream &));

        /**
        * \brief << operator allowing to log elements that ostream actually
        * accepts (char, int...).
        * \tparam T the type of element we want to log.
        * \param[in] val the element we want to log.
        * \return the same logger to be able to stream several things
        * (e.g. logger<<elA<<elB).
        */
        template<typename T>
        Logger operator<<(const T &val) {
            *out << val;

            // flushes the buffer, useful especially with ofstream where without that, nothing will be printed until close
            out->flush();

            return *this;
        }
    };
}
#endif
