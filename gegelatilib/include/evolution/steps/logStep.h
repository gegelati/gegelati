#ifndef EVOLUTION_LOG_STEP_H
#define EVOLUTION_LOG_STEP_H

#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "individual.h"
#include "evolution/module.h"
#include "util/stopwatch.h"

namespace Evolution {

    /**
     * \brief Step writing the progress of the evolution: one row per generation, made of columns.
     *
     * A row starts with the generation, then has one value per column given, then the time since the previous row and since the start of the
     * stopwatch of the evolution (EvolutionData::stopwatch, started with the data, so the time of the generation 0 is the one of the initialization).
     * The initialization of the step writes the row of the initial state, the generation 0, so the log starts with the initial population.
     * The header is written before the row of generation 0, and before the first row written by the step.
     *
     * A column is a name and a function computing a number from the data, and the names of the data it reads. These are required by the
     * log step, like the data of any step, so the evolution checks that an initialization or a previous step provides them and the
     * function can read them without testing they exist, except in the row of the initial state where a column whose data do not exist yet
     * (e.g. the offspring) is written "-". A value that is not a number (e.g. a fitness that was not
     * computed during the generation, like a periodic validation) is written "-". Ready-made columns exist for the usual data
     * (sizeColumn, fitnessColumns, stepTimeColumns) and the presets list their own (BasicEvolutionConfig::logColumns).
     *
     * By default the values are aligned in columns of a given width, to read them in a console. With a separator (e.g. ";") they are
     * only separated by it, to write a file (e.g. a CSV) read by another tool. Several log steps can write to different streams.
     *
     * \code
     * std::vector<LogStep::Column> columns = evolution.logColumns();
     * columns.push_back({"Offspring", [offspring](const EvolutionData& data) { return data.get<std::vector<std::shared_ptr<Individual>>>(offspring)->size(); }, 0, {offspring}});
     * module->add(std::make_shared<LogStep>(std::cout, columns));
     * module->add(std::make_shared<LogStep>(file, stepTimeColumns(*module), ";"));
     * \endcode
     */
    class LogStep : public Module {
        public:
            /// Column of the log.
            struct Column {
                /// Heading of the column.
                std::string name;

                /// Function computing the value of the column from the data, not a number if there is none.
                std::function<double(const EvolutionData&)> value;

                /// Number of digits after the decimal point, zero for counts.
                int precision = 2;

                /// Names of the data read by the function, required by the log step so that the evolution checks they are provided.
                std::set<std::string> required = {};
            };

        protected:
            /// Stream receiving the log.
            std::ostream* output;

            /// Columns written after the generation.
            std::vector<Column> columns;

            /// Separator of the values, empty to align them in columns of columnWidth characters.
            std::string separator;

            /// Width of the columns when there is no separator.
            size_t columnWidth = 14;

            /// Time of the evolution at the previous row, to give the time of each generation.
            double previousTime = 0.0;

            /// Indicates if the header was written.
            bool headerWritten = false;

            /**
             * \brief Text of a value with a given number of digits after the decimal point, "-" if it is not a number.
             *
             * \param[in] value the value.
             * \param[in] precision number of digits after the decimal point.
             */
            static std::string format(double value, int precision);

            /// Write a line made of the given texts.
            void writeLine(const std::vector<std::string>& texts);

            /// Write the row of a generation, with the header before the first row and the row of the generation 0.
            void writeRow(const EvolutionData& state, uint64_t generation);

        public:
            /**
             * \brief Constructor.
             *
             * \param[in] output stream receiving the log, it must outlive the step.
             * \param[in] columns columns written after the generation.
             * \param[in] separator separator of the values, empty to align the values in columns (see setColumnWidth).
             * \param[in] name name of the step.
             */
            LogStep(std::ostream& output, std::vector<Column> columns, std::string separator = "", std::string name = "Log");

            /**
             * \brief Set the width of the columns, used when there is no separator.
             *
             * \param[in] width number of characters of each column.
             */
            void setColumnWidth(size_t width) { this->columnWidth = width; }

            /// Inherited from Module
            virtual void apply(EvolutionData& state) override;
    };

    /**
     * \brief Column showing the number of individuals of a list (a population, the offspring...).
     *
     * \param[in] individuals name of the list of individuals.
     * \param[in] name heading of the column.
     */
    LogStep::Column sizeColumn(const std::string& individuals, std::string name = "Pop_Size");

    /**
     * \brief Fitness of the individuals of a list that have one.
     *
     * \param[in] state the data of the evolution.
     * \param[in] individuals name of the list of individuals.
     * \param[in] fitness name of the fitness of the individuals.
     * \return the fitness of the individuals, empty if it was not computed during the current generation.
     */
    std::vector<double> fitnessOf(const EvolutionData& state, const std::string& individuals, const std::string& fitness);

    /**
     * \brief Columns showing the minimum, the average and the maximum of a fitness over a list of individuals.
     *
     * The values are not shown when the fitness was not computed during the current generation.
     *
     * \param[in] prefix prefix of the headings, e.g. "Fit" gives Fit_Min, Fit_Avg and Fit_Max.
     * \param[in] individuals name of the list of individuals.
     * \param[in] fitness name of the fitness of the individuals.
     */
    std::vector<LogStep::Column> fitnessColumns(const std::string& prefix, const std::string& individuals,
                                                const std::string& fitness);

    /**
     * \brief Columns showing the duration in seconds of the last run of each step of a module (see Module::getLastDuration).
     *
     * There is one column per step that holds no other step, in the module and the modules it holds at the time of the call.
     * The log step must come after them to show the current generation.
     *
     * \param[in] module the module whose steps are timed.
     */
    std::vector<LogStep::Column> stepTimeColumns(const Module& module);

}; // namespace Evolution

#endif // EVOLUTION_LOG_STEP_H
