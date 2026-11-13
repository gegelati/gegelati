#ifndef EVOLUTION_ASSIGN_FITNESS_STEP_H
#define EVOLUTION_ASSIGN_FITNESS_STEP_H

#include <memory>
#include <set>
#include <string>

#include "metrics/metricMap.h"
#include "evolution/module.h"
#include "fitnessAssignment/assigner.h"

namespace Evolution {

    /**
     * \brief Step converting metrics into fitness.
     */
    class AssignFitnessStep : public Module {
        protected:
            /// Name of the metrics read.
            std::string metrics;

            /// Name of the computed fitness.
            std::string fitness;

            /// Name of the assigner computing the fitness, a component of the data (FitnessAssignment::Assigner).
            std::string assigner;

        public:
            /**
             * \brief Constructor.
             *
             * \param[in] metrics name of the metrics read.
             * \param[in] fitness name of the computed fitness.
             * \param[in] assigner name of the assigner computing the fitness, stored in the data as a FitnessAssignment::Assigner.
             * \param[in] name name of the step.
             */
            AssignFitnessStep(std::string metrics, std::string fitness, std::string assigner, std::string name = "AssignFitness")
                : Module(std::move(name)), metrics{std::move(metrics)}, fitness{std::move(fitness)}, assigner{std::move(assigner)}
            {
                this->require(this->metrics);
                this->require(this->assigner);
                this->provide(this->fitness);
            }

            /// Return the name of the computed fitness.
            const std::string& getFitness() const { return this->fitness; }

            /// Inherited from Module
            virtual std::string details() const override;
            /// Inherited from Module
            virtual void apply(EvolutionData& state) override;
    };

}; // namespace Evolution

#endif // EVOLUTION_ASSIGN_FITNESS_STEP_H
