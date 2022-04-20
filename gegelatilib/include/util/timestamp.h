#ifndef TIMESTAMP_H
#define TIMESTAMP_H

#include <string>

namespace Util {
    /**
     * \brief Utility function to get a timestamp.
     *
     * Returns a string containing the current date and time using the following
     * format: YYYY-MM-DD HH:MM:SS
     */
    std::string getCurrentDate();

} // namespace Util

#endif