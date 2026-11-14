#ifndef EVOLUTION_LOG_COLUMN_STEP_H
#define EVOLUTION_LOG_COLUMN_STEP_H

#include <functional>
#include <set>
#include <string>

#include "evolution/module.h"

namespace Evolution {

    /**
     * \brief Step computing one column of a log: a number computed from the data at each generation (see LogModule).
     *
     * The data read by the function are required by the step, like the data of any step, so the evolution checks that they are provided.
     * The value is not a number, written "-", when the data do not exist yet (e.g. the offspring in the row of the initial state), when the
     * function returns NaN (e.g. a fitness not computed during the generation), or when the step did not run during the generation (see setPeriod).
     */
    class LogColumnStep : public Module {
        protected:
            /// Function computing the value of the column from the data, NaN if there is none.
            std::function<double(const EvolutionData&)> value;

            /// Number of digits after the decimal point, zero for counts.
            int precision;

            /// Value computed by the last run of the step.
            double lastValue = 0.0;

            /// Generation of the last run of the step, to tell whether the value is the one of the current generation.
            uint64_t lastGeneration = 0;

            /// Indicates if the step ran at least once.
            bool hasRun = false;

        public:
            /**
             * \brief Constructor.
             *
             * \param[in] name heading of the column.
             * \param[in] value function computing the value of the column from the data, NaN if there is none.
             * \param[in] precision number of digits after the decimal point, zero for counts.
             * \param[in] required names of the data read by the function.
             */
            LogColumnStep(std::string name, std::function<double(const EvolutionData&)> value, int precision = 2, std::set<std::string> required = {});

            /**
             * \brief Text of a value with a given number of digits after the decimal point, "-" if it is not a number.
             *
             * \param[in] value the value.
             * \param[in] precision number of digits after the decimal point.
             */
            static std::string format(double value, int precision);

            /**
             * \brief Return the text of the value of the column for the current generation, "-" if there is none.
             *
             * \param[in] state the data of the evolution.
             */
            std::string text(const EvolutionData& state) const;

            /// Inherited from Module, computes the value of the column.
            virtual void apply(EvolutionData& state) override;
    };

}; // namespace Evolution

#endif // EVOLUTION_LOG_COLUMN_STEP_H
