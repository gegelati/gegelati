#ifndef EVOLUTION_STEP_H
#define EVOLUTION_STEP_H

#include <set>
#include <stdexcept>
#include <string>
#include <utility>

#include "evolution/evolutionState.h"

namespace Evolution {

    /**
     * \brief One stage of an evolution (selection, mutation, evaluation, logging...).
     *
     * A step reads and updates entries of the EvolutionState. Steps are chained by a Pipeline,
     * so a new evolutionary algorithm is a new list of steps, not a new Module.
     *
     * A step is identified by its address: pipelines are edited with the step objects themselves.
     * Its name is only a label for summaries, timings and error messages, and several steps may share it.
     */
    class Step {
        protected:
            /// Label of the step.
            std::string stepName;

            /// Number of generations between two runs of the step in a generation pipeline.
            size_t period = 1;

            /**
             * \brief Constructor of the base class.
             *
             * \param[in] name name of the step.
             */
            explicit Step(std::string name) : stepName{std::move(name)} {}

        public:
            /// Default polymorphic destructor
            virtual ~Step() = default;

            // Disable copying to avoid accidental copies (use shared pointers instead).
            Step(const Step&) = delete;
            Step& operator=(const Step&) = delete;

            /// Return the name of the step.
            const std::string& name() const { return this->stepName; }

            /**
             * \brief Rename the step, e.g. to tell apart two steps of the same kind in a summary.
             *
             * \param[in] name the new name.
             */
            void setName(std::string name) { this->stepName = std::move(name); }

            /**
             * \brief Run the step only once every few generations (e.g. a costly evaluation).
             *
             * The step runs on generations 0, period, 2 * period... and always during the initialization.
             * Entries written by the step are therefore older than the current generation most of the time
             * (see EvolutionState::isUpToDate).
             *
             * \param[in] period number of generations between two runs.
             * \throw std::runtime_error if the period is zero.
             */
            void setPeriod(size_t period)
            {
                if(period == 0) {
                    throw std::runtime_error("Evolution::Step::setPeriod: the period must be positive");
                }
                this->period = period;
            }

            /// Return the number of generations between two runs of the step.
            size_t getPeriod() const { return this->period; }

            /**
             * \brief Return true if the step runs during a generation.
             *
             * \param[in] generation the generation, EvolutionState::initGeneration for the initialization.
             */
            bool runsAt(uint64_t generation) const
            {
                return generation == EvolutionState::initGeneration || generation % this->period == 0;
            }

            /// Return the data of the state that must be filled before this step runs.
            virtual std::set<std::string> requiredData() const { return {}; }

            /// Return the data of the state filled or updated by this step.
            virtual std::set<std::string> providedData() const { return {}; }

            /// Return an optional one line description of the parameters of the step, used in summaries.
            virtual std::string details() const { return ""; }

            /**
             * \brief Run the step.
             *
             * \param[in,out] state the state of the evolution read and updated by the step.
             */
            virtual void apply(EvolutionState& state) = 0;
    };

}; // namespace Evolution

#endif // EVOLUTION_STEP_H
