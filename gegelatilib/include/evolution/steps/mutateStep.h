#ifndef EVOLUTION_MUTATE_STEP_H
#define EVOLUTION_MUTATE_STEP_H

#include <memory>
#include <set>
#include <string>

#include "individual.h"
#include "evolution/module.h"
#include "graphBased/genotypeGenerator.h"
#include "mutation/mutator.h"

namespace Evolution {

    /**
     * \brief Step mutating individuals, with the generator found in the data.
     */
    class MutateStep : public Module {
        protected:
            /// Name of the mutated individuals.
            std::string individuals;

            /// Name of the mutator applied to the individuals, a component of the data (Mutation::Mutator).
            std::string mutator;

            /// Name of the generator of the genotypes values.
            std::string generator;

            /// Maximum number of threads used to mutate.
            size_t nbThreads;

        public:
            /**
             * \brief Constructor.
             *
             * \param[in] individuals name of the mutated individuals, a list of mutable individuals (e.g. offspring).
             * \param[in] mutator name of the mutator applied to the individuals, stored in the data as a Mutation::Mutator.
             * \param[in] generator name of the generator of the genotypes values.
             * \param[in] nbThreads maximum number of threads used to mutate.
             * \param[in] name name of the step.
             */
            MutateStep(std::string individuals, std::string mutator, std::string generator,
                       size_t nbThreads = 1, std::string name = "Mutate");

            /// Return the name of the mutated individuals.
            const std::string& getIndividuals() const { return this->individuals; }

            /// Return the maximum number of threads used to mutate.
            size_t getNbThreads() const { return this->nbThreads; }

            /// Inherited from Module
            virtual std::string details() const override;
            /// Inherited from Module
            virtual void apply(EvolutionData& state) override;
    };

}; // namespace Evolution

#endif // EVOLUTION_MUTATE_STEP_H
