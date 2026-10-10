#ifndef DEFAULT_LOGGER_H
#define DEFAULT_LOGGER_H

#include <iomanip>
#include "log/logger.h"

namespace Log {

    /**
     * Writes generic, fixed-width table rows. The logger does not interpret
     * column names or values; callers define the table and populate each row.
     */
    class Basiclogger : public Logger
    {
      protected:
        /// @brief todo  
        static constexpr int columnWidth = 9;

      public:
        /// @brief constructor
        explicit Basiclogger(std::ostream& output = std::cout) : Logger(output) {
            // fixing float precision
            *this->output << std::setprecision(2) << std::fixed << std::right;
        }


        /// Write a column heading.
        void addHeaderValue(const std::string& name)
        {
            *this->output << std::setw(columnWidth) << name;
        }

        /// @brief multiple values
        void addHeaderValues(const std::vector<std::string>& names)
        {
            for(const std::string& name: names) {
                this->addHeaderValue(name);
            }
        }

        /// Write a value in the current row.
        template <typename T> void addGenerationValue(const T& value)
        {
            *this->output << std::setw(columnWidth) << value;
        }
        
        /// Write multiple values value in the current row.
        template <typename T> void addGenerationValues(const std::vector<T>& values)
        {
            for(const T& value: values) {
                this->addGenerationValue(value);
            }
        }

        virtual void initializeModule() {
            *this->output << std::endl;
        };

        /// @brief Method runed after every generation
        virtual void endGeneration() {
            *this->output << std::endl;
        };

        /// @brief Method for logger ending
        virtual void endModule() {
            *this->output << "Evolution, over." << std::endl;
        };
    };
} // namespace Log

#endif // DEFAULT_LOGGER_H
