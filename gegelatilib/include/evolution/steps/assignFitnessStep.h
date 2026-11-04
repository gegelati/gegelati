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
            Key<std::map<uint64_t, std::unique_ptr<Metrics::MetricMap>>> metrics;

            /// Name of the computed fitness.
            Key<std::map<uint64_t, double>> fitness;

            /// Assigner computing the fitness.
            std::shared_ptr<const FitnessAssignment::Assigner> assigner;

            /// Hash of the metric from which the fitness is computed.
            uint64_t metricHash;

        public:
            /**
             * \brief Constructor.
             *
             * \param[in] metrics key of the metrics read.
             * \param[in] fitness key of the computed fitness.
             * \param[in] assigner assigner computing the fitness.
             * \param[in] metricHash hash of the metric from which the fitness is computed.
             * \param[in] name name of the step.
             */
            AssignFitnessStep(Key<std::map<uint64_t, std::unique_ptr<Metrics::MetricMap>>> metrics, Key<std::map<uint64_t, double>> fitness, std::shared_ptr<const FitnessAssignment::Assigner> assigner, uint64_t metricHash,
                              std::string name = "AssignFitness")
                : Module(std::move(name)), metrics{std::move(metrics)}, fitness{std::move(fitness)}, assigner{std::move(assigner)}, metricHash{metricHash}
            {
                this->require(this->metrics);
                this->provide(this->fitness);
            }

            /// Return the key of the computed fitness.
            const Key<std::map<uint64_t, double>>& getFitness() const { return this->fitness; }

            /// Inherited from Module
            virtual std::string details() const override;
            /// Inherited from Module
            virtual void apply(EvolutionData& state) override;
    };

}; // namespace Evolution

#endif // EVOLUTION_ASSIGN_FITNESS_STEP_H
