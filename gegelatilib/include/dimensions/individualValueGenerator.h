#ifndef INDIVIDUAL_VALUE_GENERATOR_H
#define INDIVIDUAL_VALUE_GENERATOR_H

#include "dimensions/dataValueGenerator.h"

#include "graphBased/genotypeGenerator.h"
#include "mutation/mutator.h"
#include "representations/representation.h"

namespace Dimensions {

    /**
     * \brief Generator of individuals as values of nodes (e.g. the members of a TPG): a new value is a random individual
     * of a representation, and a mutation mutates the genotype of the individual.
     */
    class IndividualValueGenerator : public DataValueGenerator {
        protected:
            /// Representation of the mutated individual
            std::shared_ptr<const Representations::Representation> representation;

            /// @brief Generator used to create the individual, made from the constraint of the representation
            std::shared_ptr<const GraphBased::GenotypeGenerator> generator;

            /// Mutation engine to mutate the genotype
            const Mutation::Mutator& mutator;

        public:
            /**
             * \brief Constructor for IndividualValueGenerator.
             */
            IndividualValueGenerator(std::shared_ptr<const Representations::Representation> representation, const Mutation::Mutator& mutator)
            : representation(std::move(representation)), generator{std::make_shared<const GraphBased::GenotypeGenerator>(this->representation->getGenotypeConstraint())},
              mutator(mutator) {}

            /**
             * \brief Create a random individual of the representation.
             */
            virtual Data::DataValue initialize(RNG::RNG& rng) const override {
                std::shared_ptr<const Individual> indiv = std::make_shared<Individual>(
                    this->representation,
                    this->mutator.initRandomGenotype(*this->generator, rng));
                return Data::DataValue::scalar<std::shared_ptr<const Individual>>(indiv);
            }

            virtual Data::DataValue mutate(const Data::DataValue& currentValue, RNG::RNG& rng) const override {
                std::shared_ptr<const Individual> currentIndiv = currentValue.getScalar<std::shared_ptr<const Individual>>();
                std::shared_ptr<const Individual> mutatedIndiv = std::make_shared<Individual>(
                    this->representation,
                    this->mutator.mutateGenotype(currentIndiv->getGenotype(), *this->generator, rng));
                return Data::DataValue::scalar<std::shared_ptr<const Individual>>(mutatedIndiv);
            }

            /// @brief Inherrit from DataValueGenerator class 
            virtual std::unique_ptr<DataValueGenerator> cloneUniquePtr() const override {
                return std::make_unique<IndividualValueGenerator>(this->representation, this->mutator);
            }
    };
};

#endif // INDIVIDUAL_VALUE_GENERATOR_H