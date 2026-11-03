#ifndef EVOLUTION_MUTATE_STEP_H
#define EVOLUTION_MUTATE_STEP_H

#include <memory>
#include <set>
#include <string>

#include "evolution/module.h"
#include "graphBased/genotypeGenerator.h"
#include "mutation/mutator.h"

namespace Evolution {

    /**
     * \brief Step mutating individuals.
     */
    class MutateStep : public Module {
        protected:
            /// Name of the mutated individuals.
            Key<std::vector<std::shared_ptr<Individual>>> individuals;

            /// Mutator applied to the individuals.
            std::shared_ptr<const Mutation::Mutator> mutator;

            /// Generator of the genotypes values.
            std::shared_ptr<const GraphBased::GenotypeGenerator> generator;

            /// Maximum number of threads used to mutate.
            size_t nbThreads;

        public:
            /**
             * \brief Constructor.
             *
             * \param[in] individuals name of the mutated individuals, a list of mutable individuals (e.g. offspring).
             * \param[in] mutator mutator applied to the individuals.
             * \param[in] generator generator of the genotypes values.
             * \param[in] nbThreads maximum number of threads used to mutate.
             * \param[in] name name of the step.
             * \throw std::runtime_error if the mutator or the generator is null.
             */
            MutateStep(Key<std::vector<std::shared_ptr<Individual>>> individuals, std::shared_ptr<const Mutation::Mutator> mutator, std::shared_ptr<const GraphBased::GenotypeGenerator> generator,
                       size_t nbThreads = 1, std::string name = "Mutate");

            /// Return the name of the mutated individuals.
            const Key<std::vector<std::shared_ptr<Individual>>>& getIndividuals() const { return this->individuals; }

            /// Return the maximum number of threads used to mutate.
            size_t getNbThreads() const { return this->nbThreads; }

            /// Inherited from Module
            virtual std::set<std::string> requiredData() const override;
            /// Inherited from Module
            virtual std::string details() const override;
            /// Inherited from Module
            virtual void apply(EvolutionData& state) override;
    };

}; // namespace Evolution

#endif // EVOLUTION_MUTATE_STEP_H
