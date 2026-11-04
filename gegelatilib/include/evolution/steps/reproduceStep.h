#ifndef EVOLUTION_REPRODUCE_STEP_H
#define EVOLUTION_REPRODUCE_STEP_H

#include <memory>
#include <set>
#include <string>

#include "individual.h"
#include "evolution/module.h"
#include "reproduction/reproducer.h"

namespace Evolution {

    /**
     * \brief Step creating offspring from parents.
     */
    class ReproduceStep : public Module {
        protected:
            /// Name of the parents.
            Key<std::vector<std::shared_ptr<Individual>>> parents;

            /// Name of the created offspring.
            Key<std::vector<std::shared_ptr<Individual>>> offspring;

            /// Reproducer creating the offspring.
            std::shared_ptr<const Reproduction::Reproducer> reproducer;

            /// Number of offspring created.
            size_t nbOffspring;

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
            ReproduceStep(Key<std::vector<std::shared_ptr<Individual>>> parents, std::shared_ptr<const Reproduction::Reproducer> reproducer, size_t nbOffspring, Key<std::vector<std::shared_ptr<Individual>>> offspring,
                          std::string name = "Reproduce")
                : Module(std::move(name)), parents{std::move(parents)}, reproducer{std::move(reproducer)}, nbOffspring{nbOffspring}, offspring{std::move(offspring)}
            {
                this->require(this->parents);
                this->provide(this->offspring);
            }

            /// Inherited from Module
            virtual std::string details() const override;
            /// Inherited from Module
            virtual void apply(EvolutionData& state) override;
    };

}; // namespace Evolution

#endif // EVOLUTION_REPRODUCE_STEP_H
