#ifndef EVOLUTION_INIT_POPULATION_STEP_H
#define EVOLUTION_INIT_POPULATION_STEP_H

#include <memory>
#include <set>
#include <string>

#include "representations/representation.h"
#include "evolution/module.h"
#include "graphBased/genotypeGenerator.h"
#include "mutation/mutator.h"

namespace Evolution {

    /**
     * \brief Step creating the random individuals of a population.
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

            /// Generator of the genotypes.
            std::shared_ptr<const GraphBased::GenotypeGenerator> generator;

        public:
            /**
             * \brief Constructor.
             *
             * \param[in] individuals key of the created individuals.
             * \param[in] representation representation of the individuals, kept alive by the step.
             * \param[in] size number of individuals created.
             * \param[in] mutator mutator creating the random genotypes.
             * \param[in] generator generator of the genotypes, the one of the representation if null.
             * \param[in] name name of the step.
             * \throw std::runtime_error if the representation or the mutator is null.
             */
            InitPopulationStep(Key<std::vector<std::shared_ptr<Individual>>> individuals, std::shared_ptr<const Representations::Representation> representation, size_t size,
                               std::shared_ptr<const Mutation::Mutator> mutator, std::shared_ptr<const GraphBased::GenotypeGenerator> generator = nullptr,
                               std::string name = "InitPopulation");

            /// Inherited from Module
            virtual std::set<std::string> providedData() const override;
            /// Inherited from Module
            virtual std::string details() const override;
            /// Inherited from Module
            virtual void apply(EvolutionData& state) override;
    };

}; // namespace Evolution

#endif // EVOLUTION_INIT_POPULATION_STEP_H
