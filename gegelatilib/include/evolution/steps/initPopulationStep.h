#ifndef EVOLUTION_INIT_POPULATION_STEP_H
#define EVOLUTION_INIT_POPULATION_STEP_H

#include <memory>
#include <set>
#include <string>

#include "representations/representation.h"
#include "individual.h"
#include "evolution/module.h"
#include "graphBased/genotypeGenerator.h"
#include "mutation/mutator.h"

namespace Evolution {

    /**
     * \brief Step creating the random individuals of a population, with the generator found in the data.
     */
    class InitPopulationStep : public Module {
        protected:
            /// Individuals created.
            std::string individuals;

            /// Name of the representation of the individuals, a component of the data (const Representations::Representation).
            std::string representation;

            /// Number of individuals created.
            size_t size;

            /// Name of the mutator creating the random genotypes, a component of the data (Mutation::Mutator).
            std::string mutator;

            /// Name of the generator of the genotypes.
            std::string generator;

        public:
            /**
             * \brief Constructor.
             *
             * \param[in] individuals name of the created individuals.
             * \param[in] representation name of the representation of the individuals, stored in the data as a const Representations::Representation.
             * \param[in] size number of individuals created.
             * \param[in] mutator name of the mutator creating the random genotypes, stored in the data as a Mutation::Mutator.
             * \param[in] generator name of the generator of the genotypes.
             * \param[in] name name of the step.
             */
            InitPopulationStep(std::string individuals, std::string representation, size_t size,
                               std::string mutator, std::string generator, std::string name = "InitPopulation");

            /// Inherited from Module
            virtual std::string details() const override;
            /// Inherited from Module
            virtual void apply(EvolutionData& state) override;
    };

}; // namespace Evolution

#endif // EVOLUTION_INIT_POPULATION_STEP_H
