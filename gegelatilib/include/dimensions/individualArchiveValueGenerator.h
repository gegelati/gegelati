#ifndef INDIVIDUAL_VALUE_ARCHIVE_GENERATOR_H
#define INDIVIDUAL_VALUE_ARCHIVE_GENERATOR_H

#include "dimensions/individualValueGenerator.h"

#include "evaluation/dataset.h"

namespace Dimensions {

    /**
     * \brief Generator of individuals (e.g. the members of a TPG) whose mutated individuals behave differently, on the
     * inputs of the archive, from all the members whose answers are stored in the archive.
     */
    class IndividualValueArchiveGenerator : public IndividualValueGenerator {
        protected:
            /// Archive on which the individual novelty is measured
            const Evaluation::DataSet& archive;

        public:
            /**
             * \brief Constructor for IndividualValueArchiveGenerator.
             */
            IndividualValueArchiveGenerator(const Evaluation::DataSet& archive, std::shared_ptr<const Representations::Representation> representation, const Mutation::Mutator& mutator)
            : IndividualValueGenerator(std::move(representation), mutator), archive{archive} {}

            /**
             * \brief Mutate the individual until its answers on the archive differ from the ones stored in the archive.
             */
            virtual Data::DataValue mutate(const Data::DataValue& currentValue, RNG::RNG& rng) const override {
                std::shared_ptr<const Individual> currentIndiv = currentValue.getScalar<std::shared_ptr<const Individual>>();
                
                size_t nbAnswers = (this->archive.size() > 0) ? this->archive.getOutputAt(0).getDimensions()[0] : 0;
                std::unique_ptr<GraphBased::Genotype> genotype;
                bool isUnique = false;
                while(!isUnique) {
                    genotype = this->mutator.mutateGenotype(currentIndiv->getGenotype(), *this->generator, rng);
                    this->representation->detectIntronNodes(*genotype);

                    std::set<size_t> remainingAnswers;
                    for(size_t idxAnswer = 0; idxAnswer < nbAnswers; idxAnswer ++) {
                        remainingAnswers.insert(idxAnswer);
                    }

                    for(size_t idxInput = 0; idxInput < this->archive.size() && !remainingAnswers.empty(); idxInput++) {
                        double currentOutput = this->representation->execute(*genotype, this->archive.getInputsAt(idxInput)).getScalar<double>();
                        auto archiveOutputs = this->archive.getOutputAt(idxInput).getData<double>();
                        for(auto it = remainingAnswers.begin(); it != remainingAnswers.end(); ){
                            if(currentOutput == archiveOutputs[*it]) {
                                it++;
                            } else {
                                it = remainingAnswers.erase(it);
                            }
                        }
                    }

                    isUnique = remainingAnswers.empty();
                }
                std::shared_ptr<const Individual> mutatedIndiv = std::make_shared<Individual>(this->representation, std::move(genotype));
                return Data::DataValue::scalar<std::shared_ptr<const Individual>>(mutatedIndiv);
            }

            /// @brief Inherrit from DataValueGenerator class 
            virtual std::unique_ptr<DataValueGenerator> cloneUniquePtr() const override {
                return std::make_unique<IndividualValueArchiveGenerator>(this->archive, this->representation, this->mutator);
            }
    };
};

#endif // INDIVIDUAL_VALUE_ARCHIVE_GENERATOR_H