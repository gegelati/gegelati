#ifndef EVOLUTION_LOG_STEP_H
#define EVOLUTION_LOG_STEP_H

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "evolution/module.h"
#include "evolution/steps/logColumnStep.h"

namespace Evolution {

    /**
     * \brief Module writing the progress of the evolution: one row per generation, made of the columns it holds (see LogColumnStep).
     *
     * The initialization of the module writes the header (the headings of the columns) and the row of the initial state, the generation 0.
     * At each generation, the columns compute their values, then the row is written.
     *
     * The columns are modules: they are added, found, replaced and removed by name like any step (their name is their heading), and can
     * run only every few generations. The usual log of a population (generation, size, fitness and times) is built by BasicLogConfig.
     *
     * By default the values are aligned in columns of a given width, to read them in a console. With a separator (e.g. ";") they are
     * only separated by it, to write a file (e.g. a CSV) read by another tool. Several logs can write to different streams.
     *
     * \code
     * std::shared_ptr<LogModule> log = BasicLogConfig().buildModule();
     * // nbOffspring(data, offspring) returns the number of offspring.
     * log->insertBefore("T_Gen", std::make_shared<LogColumnStep>("Offspring", [offspring](const EvolutionData& data) { return nbOffspring(data, offspring); }, 0, std::set<std::string>{offspring}));
     * module->addSubModule(log);
     * \endcode
     */
    class LogModule : public Module {
        protected:
            /// Stream receiving the log.
            std::ostream* output;

            /// Separator of the values, empty to align them in columns of columnWidth characters.
            std::string separator;

            /// Width of the columns when there is no separator.
            size_t columnWidth = 14;

            /// Write a line made of the given texts.
            void writeLine(const std::vector<std::string>& texts);

            /// Write the header: the headings of the columns.
            void writeHeader();

            /// Write the row of the current generation, with the values computed by the columns.
            void writeRow(const EvolutionData& state);

            /// Write the header, then the row of the initial state (the initialization of the module).
            void writeInitialRows(EvolutionData& state);

        public:
            /**
             * \brief Constructor, without columns.
             *
             * \param[in] output stream receiving the log, it must outlive the module.
             * \param[in] separator separator of the values, empty to align the values in columns (see setColumnWidth).
             * \param[in] name name of the module.
             */
            LogModule(std::ostream& output, std::string separator = "", std::string name = "Log");

            /**
             * \brief Set the width of the columns, used when there is no separator.
             *
             * \param[in] width number of characters of each column.
             */
            void setColumnWidth(size_t width) { this->columnWidth = width; }

            /// Inherited from Module: computes the columns, then writes the row.
            virtual void apply(EvolutionData& state) override;
    };

}; // namespace Evolution

#endif // EVOLUTION_LOG_STEP_H
