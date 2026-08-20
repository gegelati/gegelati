

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
        std::map<size_t, std::vector<std::reference_wrapper<const Data::DataHandler>>> inputsExtracted;

      public:


        /**
         * \brief Destructor of the class.
         *
         * In addition to default behavior, free all the memory associated to the
         * referenced DataHandler in the dataHandlers attribute.
         */
        ~ArchiveMetric();

        /**
         * \brief Constructor.
         * 
         * \param[in] extractionProbability probability of extracting an input source.
         * \param[in] seed seed used to initialized the deterministic RNG
         */
        ArchiveMetric(double extractionProbability, size_t seed)
            : EvaluationMetric(0.0), extractionProbability{extractionProbability} {
                // set deterministic seed to the local rng.
                rng.setSeed(seed);
            };

        /**
         * \brief Combien the hash of a set of dataHandlers into a single one.
         *
         * Hashes of each DataHandler is accessed with the
         * DataHandler::getHash() method.
         *
         * \return the hash resulting from the combination.
         */
        static size_t getCombinedHash(
            const std::vector<std::reference_wrapper<const Data::DataHandler>>&
                dHandler);

        /**
         * Return the inputs extracted.
         */
        virtual const std::map<size_t, std::vector<std::reference_wrapper<const Data::DataHandler>>>&  getInputsExtracted() const;

        /**
         * \brief Extract metrics from the individual in the learning environment.
         *
         * This method is called at every step of the environment evaluation.
         * 
         * With defined probability, it extracts and copies the current input dataSources of the environment.
         *
         * \param[in] individual the individual performing a step.
         * \param[in] actionValues the action values taken by the individual.
         * \param[in] learningEnvironment the learning environment in which the
         * individual is evaluated.
         */
        virtual void extractMetricsStep(
            const Evolution::Individual& individual, std::vector<double> actionValues,
            const Learn::LearningEnvironment& learningEnvironment) override;
    };


}; // namespace Evaluation

#endif // ARCHIVE_METRICS_H