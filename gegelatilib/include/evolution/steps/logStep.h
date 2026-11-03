#ifndef EVOLUTION_LOG_STEP_H
#define EVOLUTION_LOG_STEP_H

#include <functional>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "evolution/module.h"
#include "util/stopwatch.h"

namespace Evolution {

    /**
     * \brief Step writing the progress of the evolution of a list of individuals as a fixed-width table.
     *
     * Added at the end of the generation steps, it writes one row per generation: generation, integer columns
     * (number of individuals by default), min/average/max of each logged fitness of the individuals, then the time since the
     * previous row (the generation, if the step is the last one), and since the initialization of the step. The step measures
     * these times itself with a Stopwatch started by its initialization. The header of the table
     * is written before the row of generation 0 (and before the first row written by the step).
     *
     * Fitness columns show "-" when the fitness was not computed during the generation (e.g. a periodic
     * validation), instead of repeating older values.
     */
    class LogStep : public Module {
        public:
            /// Integer column of the table.
            struct Column {
                /// Heading of the column.
                std::string name;

                /// Function computing the value of the column from the state.
                std::function<size_t(const EvolutionData&)> value;
            };

        protected:
            /// Width of each column of the table.
            static constexpr int columnWidth = 14;

            /// Stream receiving the table.
            std::ostream* output;

            /// Name of the logged individuals.
            Key<std::vector<std::shared_ptr<Individual>>> individuals;

            /// Prefix of the headings and name of each logged fitness.
            std::vector<std::pair<std::string, Key<std::map<uint64_t, double>>>> fitnesses;

            /// Integer columns written after the generation number.
            std::vector<Column> columns;

            /// Measures the times of the table, shared with the initialization of the step that starts it.
            std::shared_ptr<Stopwatch> stopwatch;

            /// Indicates if the header was written.
            bool headerWritten = false;

            /// Write the header of the table.
            void writeHeader();

        public:
            /**
             * \brief Constructor.
             *
             * \param[in] output stream receiving the table, it must outlive the step.
             * \param[in] individuals name of the logged individuals, a list of individuals.
             * \param[in] fitnesses prefix of the headings (e.g. "Fit") and name of each logged fitness of the individuals.
             * \param[in] columns integer columns written after the generation number, the number of individuals if empty.
             * \param[in] name name of the step.
             */
            LogStep(std::ostream& output, Key<std::vector<std::shared_ptr<Individual>>> individuals, std::vector<std::pair<std::string, Key<std::map<uint64_t, double>>>> fitnesses, std::vector<Column> columns = {},
                    std::string name = "Log");

            /// Inherited from Module
            virtual std::set<std::string> requiredData() const override;
            /// Inherited from Module
            virtual void apply(EvolutionData& state) override;
    };

}; // namespace Evolution

#endif // EVOLUTION_LOG_STEP_H
