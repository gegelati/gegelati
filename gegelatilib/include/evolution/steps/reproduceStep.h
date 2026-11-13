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
            std::string parents;

            /// Name of the created offspring.
            std::string offspring;

            /// Name of the reproducer creating the offspring, a component of the data (Reproduction::Reproducer).
            std::string reproducer;

            /// Number of offspring created.
            size_t nbOffspring;

        public:
            /**
             * \brief Constructor.
             *
             * \param[in] parents name of the parents, a list of individuals.
             * \param[in] reproducer name of the reproducer creating the offspring, stored in the data as a Reproduction::Reproducer.
             * \param[in] nbOffspring number of offspring created.
             * \param[in] offspring name of the created offspring, a list of mutable individuals.
             * \param[in] name name of the step.
             */
            ReproduceStep(std::string parents, std::string reproducer, size_t nbOffspring, std::string offspring,
                          std::string name = "Reproduce")
                : Module(std::move(name)), parents{std::move(parents)}, reproducer{std::move(reproducer)}, nbOffspring{nbOffspring}, offspring{std::move(offspring)}
            {
                this->require(this->parents);
                this->require(this->reproducer);
                this->provide(this->offspring);
            }

            /// Inherited from Module
            virtual std::string details() const override;
            /// Inherited from Module
            virtual void apply(EvolutionData& state) override;
    };

}; // namespace Evolution

#endif // EVOLUTION_REPRODUCE_STEP_H
