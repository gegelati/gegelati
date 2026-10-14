#ifndef INDIVIDUAL_VALUE_GENERATOR_H
#define INDIVIDUAL_VALUE_GENERATOR_H

#include "dimensions/dataValueGenerator.h"

#include "mutation/mutator.h"
#include "representations/representation.h"

namespace Dimensions {

    /**
     * \brief Class generating a DataValue of numeric type T sampled unfiromly within a specified range.
     */
    class IndividualValueGenerator : public DataValueGenerator {
        protected:
            /// Representation of the mutated individual
            const Representations::Representation& representation;

            /// @brief Generator used to create the individual
            const GraphBased::GenotypeGenerator& generator;

            /// Mutation engine to mutate the genotype
            const Mutation::Mutator& mutator;

        public:
            /**
             * \brief Constructor for IndividualValueGenerator.
             */
            IndividualValueGenerator(const Representations::Representation& representation, const GraphBased::GenotypeGenerator& generator, const Mutation::Mutator& mutator) 
            : representation(representation), generator{generator}, mutator(mutator) {}

            /**
             * \brief Sample method creating a DataValue of type T within the specified range.
             */
            virtual Data::DataValue initialize(RNG::RNG& rng) const override {
                std::shared_ptr<const Individual> indiv = std::make_shared<Individual>(
                    representation, 
                    mutator.initRandomGenotype(generator, rng));
                return Data::DataValue::scalar<std::shared_ptr<const Individual>>(indiv);
            }

            virtual Data::DataValue mutate(const Data::DataValue& currentValue, RNG::RNG& rng) const override {
                std::shared_ptr<const Individual> currentIndiv = currentValue.getScalar<std::shared_ptr<const Individual>>();
                std::shared_ptr<const Individual> mutatedIndiv = std::make_shared<Individual>(
                    representation, 
                    mutator.mutateGenotype(currentIndiv->getGenotype(), generator, rng));
                return Data::DataValue::scalar<std::shared_ptr<const Individual>>(mutatedIndiv);
            }

            /// @brief Inherrit from DataValueGenerator class 
            virtual std::unique_ptr<DataValueGenerator> cloneUniquePtr() const override {
                return std::make_unique<IndividualValueGenerator>(this->representation, this->generator, this->mutator);
            }
    };
};

#endif // DATA_VALUE_GENERATOR_H