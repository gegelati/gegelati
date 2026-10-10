#ifndef LOGGER_H
#define LOGGER_H

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace Log {

    /**
     * Writes generic, fixed-width table rows. The logger does not interpret
     * column names or values; callers define the table and populate each row.
     */
    class Logger
    {
      protected:
        /// @brief todo
        std::ostream* output;

      public:
        // Default destructor
        virtual ~Logger() = default;

        Logger() = default;

        // Disable copying to avoid accidental copies (use references or pointers instead).
        Logger(const Logger&) = delete;
        Logger& operator=(const Logger&) = delete;

        /// @brief constructor
        explicit Logger(std::ostream& output = std::cout) : output{&output} {}


        /// Method for logger initialization run at the beginning of the module
        virtual void initializeModule() {};

        /// @brief Method runed after every generation
        virtual void endGeneration() {};

        /// @brief Method for logger ending
        virtual void endModule() {};
        
    };
} // namespace Log

#endif // LOGGER_H
