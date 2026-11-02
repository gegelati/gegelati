#ifndef EVOLUTION_REPRODUCE_STEP_H
#define EVOLUTION_REPRODUCE_STEP_H

#include <memory>
#include <set>
#include <string>

#include "evolution/step.h"
#include "reproduction/reproducer.h"

namespace Evolution {

    /**
     * \brief Step creating offspring from parents.
     */
    class ReproduceStep : public Step {
        protected:
            /// Name of the parents.
            std::string parents;

            /// Reproducer creating the offspring.
            std::shared_ptr<const Reproduction::Reproducer> reproducer;

            /// Number of offspring created.
            size_t nbOffspring;

            /// Name of the created offspring.
            std::string offspring;

        public:
            /**
             * \brief Constructor.
             *
             * \param[in] parents name of the parents, a list of individuals.
             * \param[in] reproducer reproducer creating the offspring.
             * \param[in] nbOffspring number of offspring created.
             * \param[in] offspring name of the created offspring, a list of mutable individuals.
             * \param[in] name name of the step.
             */
            ReproduceStep(std::string parents, std::shared_ptr<const Reproduction::Reproducer> reproducer, size_t nbOffspring, std::string offspring,
                          std::string name = "Reproduce")
                : Step(std::move(name)), parents{std::move(parents)}, reproducer{std::move(reproducer)}, nbOffspring{nbOffspring}, offspring{std::move(offspring)} {}

            /// Inherited from Step
            virtual std::set<std::string> requiredData() const override;
            /// Inherited from Step
            virtual std::set<std::string> providedData() const override;
            /// Inherited from Step
            virtual std::string details() const override;
            /// Inherited from Step
            virtual void apply(EvolutionState& state) override;
    };

}; // namespace Evolution

#endif // EVOLUTION_REPRODUCE_STEP_H
