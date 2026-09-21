

#ifndef ARCHIVE_METRICS_H
#define ARCHIVE_METRICS_H

#include <map>

#include "evaluation/evaluationMetric.h"
#include "mutator/rng.h"

namespace Evaluation {
    /**
     * \brief Class to extract archive metrics from the environment.
     */
    class ArchiveMetric : public EvaluationMetric
    {
      protected:

        /// @brief Probability of extracting the current input dataSources of the environment.
        double extractionProbability;

        /// @brief Local rng of the archive
        RNG::RNG rng;

        /// map of Copied datahandler of the environment containing the inputs extracted.
        std::map<size_t, std::vector<std::pair<std::unique_ptr<std::byte[]>, Data::DataType>>> inputsExtracted;

      public:

        /**
         * \brief Constructor.
         * 
         * \param[in] seed Unique seed of this metric.
         * \param[in] extractionProbability probability of extracting an input source.
         */
        ArchiveMetric(size_t seed, double extractionProbability)
            : EvaluationMetric(seed), extractionProbability{extractionProbability} {
            rng.setSeed(seed);
        };

        /**
         * \brief Print the content of the metric: its probability of sampling, and the current number of input sampled.
         */
        virtual std::string toString(std::string prefix = "") const override;


        /**
         * \brief Return the typeId of the metric
         */
        virtual std::type_index typeId() const noexcept override;

        /**
         * \brief Dupplicate the current metric with the same extractionProbability.
         * 
         * \param[in] seed Unique seed of this metric.
         */
        std::unique_ptr<EvaluationMetric> cloneEmptyUniquePtr(size_t seed) const override;

        /**
         * \brief Combien the hash of a set of dataHandlers into a single one.
         *
         * Hashes of each DataHandler is accessed with the
         * DataHandler::getHash() method.
         *
         * \return the hash resulting from the combination.
         */
        static size_t getCombinedHash(const std::vector<Data::DataView>& views);

        /**
         * Return the inputs extracted.
         */
        virtual const std::map<size_t, std::vector<std::pair<std::unique_ptr<std::byte[]>, Data::DataType>>>&  getInputsExtracted() const;



        /**
         * \brief Extract metrics from the individual in the learning environment.
         *
         * This method is called at every step of the environment evaluation.
         * 
         * With defined probability, it extracts and copies the current input dataSources of the environment.
         *
         * \param[in] individual the individual performing a step.
         * \param[in] problem the learning environment in which the
         * individual is evaluated.
         */
        virtual void extractBeforeExecution(
            const Individual& individual,
            const Evaluation::Problem& problem) override;
    };


}; // namespace Evaluation

#endif // ARCHIVE_METRICS_H