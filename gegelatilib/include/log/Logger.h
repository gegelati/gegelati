#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>

namespace Log {

    /**
     * Basic LALogger for logging generic training information.
     */
    class Logger
    {
      private:
        /**
         * Output stream where all what is logged is put.
         */
        std::ostream* out;

      public:
        /**
         * \brief Constructor initializing a specific output. Default is cout.
         *
         * \param[in] out The output stream the logger will send elements to.
         */
        explicit Logger(std::ostream& out = std::cout) : out(&out){};

        /**
         * \brief << operator to manipulate stream and enter stream-specific
         * elements (like std::endl).
         *
         * \param[in] manip The element that will be added.
         * \return The samme logger to be able to stream several things
         * (e.g. logger<<elA<<elB).
         */
        Logger operator<<(std::ostream& (*manip)(std::ostream&));

        /**
         * \brief << operator allowing to log elements that ostream actually
         * accepts (char, int...).
         *
         * \tparam T The type of element that will be logged.
         * \param[in] val The element that will be logged.
         * \return The same logger to be able to stream several things
         * (e.g. logger<<elA<<elB).
         */
        template <typename T> Logger operator<<(const T& val)
        {
            *out << val;

            // flushes the buffer, useful especially with ofstream where without
            // that, nothing will be printed until close
            out->flush();

            return *this;
        }
    };
} // namespace Log
#endif
