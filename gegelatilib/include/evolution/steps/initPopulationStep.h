#ifndef EVOLUTION_INIT_POPULATION_STEP_H
#define EVOLUTION_INIT_POPULATION_STEP_H

#include <memory>
#include <set>
#include <string>

#include "evolution/population.h"
#include "evolution/step.h"
#include "graphBased/genotypeGenerator.h"
#include "mutation/mutator.h"

namespace Evolution {

    /**
     * \brief Step creating the random individuals of a population.
     */
    class InitPopulationStep : public Step {
        protected:
            /// Created population.
            std::shared_ptr<const Population> population;

            /// Mutator creating the random genotypes.
            std::shared_ptr<const Mutation::Mutator> mutator;

            /// Generator of the genotypes.
            std::shared_ptr<const GraphBased::GenotypeGenerator> generator;

        public:
            /**
             * \brief Constructor.
             *
             * \param[in] population created population, its size and representation are used.
             * \param[in] mutator mutator creating the random genotypes.
             * \param[in] generator generator of the genotypes, the one of the representation of the population if null.
             * \param[in] name name of the step.
             * \throw std::runtime_error if the population or the mutator is null.
             */
            InitPopulationStep(std::shared_ptr<const Population> population, std::shared_ptr<const Mutation::Mutator> mutator,
                               std::shared_ptr<const GraphBased::GenotypeGenerator> generator = nullptr, std::string name = "InitPopulation");

            /// Return the created population.
            const std::shared_ptr<const Population>& getPopulation() const { return this->population; }

            /// Inherited from Step
            virtual std::set<std::string> providedData() const override;
            /// Inherited from Step
            virtual std::string details() const override;
            /// Inherited from Step
            virtual void apply(EvolutionState& state) override;
    };

}; // namespace Evolution

#endif // EVOLUTION_INIT_POPULATION_STEP_H
