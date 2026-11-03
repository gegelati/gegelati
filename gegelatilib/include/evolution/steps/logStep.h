#ifndef EVOLUTION_LOG_STEP_H
#define EVOLUTION_LOG_STEP_H

#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "evolution/module.h"
#include "util/stopwatch.h"

namespace Evolution {

    /**
     * \brief Step writing the progress of the evolution: one row per generation, made of columns.
     *
     * A row starts with the generation, then has one value per column given, then the time since the previous row and since the
     * initialization of the step. The header is written before the row of generation 0, and before the first row written by the step.
     *
     * A column is a name and a function computing a number from the data. A value that is not a number (e.g. a fitness that was not
     * computed during the generation, like a periodic validation) is written "-". Ready-made columns exist for the usual data
     * (sizeColumn, fitnessColumns, stepTimeColumns) and the presets list their own (BasicEvolution::logColumns).
     *
     * By default the values are aligned in columns of a given width, to read them in a console. With a separator (e.g. ";") they are
     * only separated by it, to write a file (e.g. a CSV) read by another tool. Several log steps can write to different streams.
     *
     * \code
     * std::vector<LogStep::Column> columns = evolution.logColumns();
     * columns.push_back({"Offspring", [offspring](const EvolutionData& data) { return data.get(offspring)->size(); }, 0});
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

            /// Measures the times of the log, shared with the initialization of the step that starts it.
            std::shared_ptr<Stopwatch> stopwatch;

            /// Indicates if the header was written.
            bool headerWritten = false;

            /// Write a line made of the given texts.
            void writeLine(const std::vector<std::string>& texts);

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
     * \param[in] individuals key of the list of individuals.
     * \param[in] name heading of the column.
     */
    LogStep::Column sizeColumn(const Key<std::vector<std::shared_ptr<Individual>>>& individuals, std::string name = "Pop_Size");

    /**
     * \brief Columns showing the minimum, the average and the maximum of a fitness over a list of individuals.
     *
     * The values are not shown when the fitness was not computed during the current generation.
     *
     * \param[in] prefix prefix of the headings, e.g. "Fit" gives Fit_Min, Fit_Avg and Fit_Max.
     * \param[in] individuals key of the list of individuals.
     * \param[in] fitness key of the fitness of the individuals.
     */
    std::vector<LogStep::Column> fitnessColumns(const std::string& prefix, const Key<std::vector<std::shared_ptr<Individual>>>& individuals,
                                                const Key<std::map<uint64_t, double>>& fitness);

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
