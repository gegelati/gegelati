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
     * \brief Step creating the random individuals of a population, with the generator found in the data (see CreateGeneratorStep).
     */
    class InitPopulationStep : public Module {
        protected:
            /// Individuals created.
            Key<std::vector<std::shared_ptr<Individual>>> individuals;

            /// Representation of the individuals.
            std::shared_ptr<const Representations::Representation> representation;

            /// Number of individuals created.
            size_t size;

            /// Mutator creating the random genotypes.
            std::shared_ptr<const Mutation::Mutator> mutator;

            /// Key of the generator of the genotypes.
            Key<GraphBased::GenotypeGenerator> generator;

        public:
            /**
             * \brief Constructor.
             *
             * \param[in] individuals key of the created individuals.
             * \param[in] representation representation of the individuals, kept alive by the step.
             * \param[in] size number of individuals created.
             * \param[in] mutator mutator creating the random genotypes.
             * \param[in] generator key of the generator of the genotypes.
             * \param[in] name name of the step.
             * \throw std::runtime_error if the representation or the mutator is null.
             */
            InitPopulationStep(Key<std::vector<std::shared_ptr<Individual>>> individuals, std::shared_ptr<const Representations::Representation> representation, size_t size,
                               std::shared_ptr<const Mutation::Mutator> mutator, Key<GraphBased::GenotypeGenerator> generator, std::string name = "InitPopulation");

            /// Inherited from Module
            virtual std::string details() const override;
            /// Inherited from Module
            virtual void apply(EvolutionData& state) override;
    };

}; // namespace Evolution

#endif // EVOLUTION_INIT_POPULATION_STEP_H
