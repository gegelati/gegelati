/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2025) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2022)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2019 - 2020)
 * Pierre-Yves Le Rolland-Raumer <plerolla@insa-rennes.fr> (2020)
 * Quentin Vacher <qvacher@insa-rennes.fr> (2023 - 2025)
 *
 * GEGELATI is an open-source reinforcement learning framework for training
 * artificial intelligence based on Tangled Program Graphs (TPGs).
 *
 * This software is governed by the CeCILL-C license under French law and
 * abiding by the rules of distribution of free software. You can use,
 * modify and/ or redistribute the software under the terms of the CeCILL-C
 * license as circulated by CEA, CNRS and INRIA at the following URL
 * "http://www.cecill.info".
 *
 * As a counterpart to the access to the source code and rights to copy,
 * modify and redistribute granted by the license, users are provided only
 * with a limited warranty and the software's author, the holder of the
 * economic rights, and the successive licensors have only limited
 * liability.
 *
 * In this respect, the user's attention is drawn to the risks associated
 * with loading, using, modifying and/or developing or reproducing the
 * software by the user in light of its specific status of free software,
 * that may mean that it is complicated to manipulate, and that also
 * therefore means that it is reserved for developers and experienced
 * professionals having in-depth computer knowledge. Users are therefore
 * encouraged to load and test the software's suitability as regards their
 * requirements in conditions enabling the security of their systems and/or
 * data to be ensured and, more generally, to use and operate it in the
 * same conditions as regards security.
 *
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL-C license and that you accept its terms.
 */

#ifndef ARCHIVE_EVAL_AGENT_H
#define ARCHIVE_EVAL_AGENT_H


#include "evaluation/evaluationAgent.h"
#include "evaluation/scoreMetric.h"
#include "evaluation/archiveMetric.h"

namespace Evaluation {


    /**
     * \brief Class used to store one recording of an Archive.
     *
     * A recording in the archive is a tuple consisting of:
     * - An ID to an individual
     * - A double resulting from the execution of the individual on the DataHandler.
     */
    typedef struct ArchiveRecording
    {
        /// ID of the individual.
        size_t individualID;

        /// Value returned by the individual for the DataHandler.
        const double result;
    } ArchiveRecording;

    /**
     * \brief Class used to control the learning steps of a Graph within
     * a given LearningEnvironment.
     */
    class ArchiveEvalAgent : public EvaluationAgent
    {
      protected:

        /// @brief Input used to give the input dimension of the archive.
        std::vector<std::reference_wrapper<const Data::DataHandler>> inputDimensions;

        /// @brief Number of input sampled stored in the archive.
        size_t archiveSize;

        /// @brief Random number generator for selecting archive inputs
        RNG::RNG rng;

        /// Vector of requested metric to measure during evaluation
        std::vector<std::unique_ptr<EvaluationMetric>> requestedMetrics;

        /// map of input and output pair values stored in the archive.
        std::vector<std::pair<std::vector<std::reference_wrapper<const Data::DataHandler>>, std::vector<ArchiveRecording>>> archive;

        /**
         * \brief set the dimension of data source by creating a copy of the given one
         * 
         * \param[in] dHandler given dataSource.
         */
        void setDimensionDataSource(std::vector<std::reference_wrapper<const Data::DataHandler>> dHandler);

      public:
        /**
         * \brief Constructor for EvaluationAgent.
         *
         * \param[in] inputDimensions example inputs given as an example of dimension.
         * \param[in] archiveSize size of the archive
         * \param[in] seed Seed for deterministic randomizer of archive selection.
         */
        ArchiveEvalAgent(std::vector<std::reference_wrapper<const Data::DataHandler>> inputDimensions, size_t archiveSize, size_t seed = 0)
            : EvaluationAgent(), archiveSize{archiveSize} {
              rng.setSeed(seed);
              this->setDimensionDataSource(inputDimensions);
            };

        /**
         * \brief Return the dataSources of the LearningEnvironment
         */
        virtual std::vector<std::reference_wrapper<const Data::DataHandler>> getDimensionsDataSources() const;

        /**
         * \brief Evaluates policy starting from the given root.
         *
         * The policy, that is, the Graph execution starting from the given
         * Vertex is evaluated nbIteration times. The generationNumber is
         * combined with the current iteration number to generate a set of
         * seeds for evaluating the policy.
         *
         * The method is const to enable potential parallel calls to it.
         *
         * \param[in] individual The individual whose genotype is evaluted.
         * \param[in] representation The representation of the individual evaluated, used to map the individual genotype to phenotype
         * \param[in] generationNumber the integer number of the current
         * generation.
         * \param[in] mode the LearningMode to use during the policy
         * evaluation.
         *
         * \return a std::shared_ptr to the EvaluationResult for the root. If
         * this root was already evaluated more times then the limit in
         * params.maxNbEvaluationPerPolicy, then the EvaluationResult from the
         * resultsPerRoot map is returned, else the EvaluationResult of the
         * current generation is returned, already combined with the
         * resultsPerRoot for this root (if any).
         */
        virtual std::shared_ptr<EvaluationResult> evaluateIndividual(
            const Evolution::Individual& individual, 
            const Evolution::Representation& representation,
            uint64_t generationNumber,
            Learn::LearningMode mode) const override;


        /**
         * \brief Evaluate all individual of the representations.
         *
         * This method calls the evaluateIndividual method for every individual
         * of the representations. The method returns a sorted map associating each
         * individual to its average score.
         *
         * \param[in] individuals The individuals whose genotypes are evaluted.
         * \param[in] representation The representation of the individuals evaluated, used to map the individual genotypes to phenotypes
         * \param[in] generationNumber the integer number of the current
         * generation.
         * \param[in] mode the LearningMode to use during the policy
         * evaluation.
         */
        virtual std::map<std::reference_wrapper<const Evolution::Individual>, 
                              std::shared_ptr<EvaluationResult>>
        evaluateIndividuals(
            const std::vector<std::reference_wrapper<const Evolution::Individual>>& individuals, 
            const Evolution::Representation& representation,
            uint64_t generationNumber,
            Learn::LearningMode mode) const override;

        /**
         * \brief Update the input used in the archive map based on the map given.
         * 
         * The map should belong to the evaluationAgent on which the archiveAgent is connected.
         * Only the EvaluationResults are used, which should contain archiveMetric.
         * 
         * Some archiveMetric are randomly sampled from the list.
         * 
         * \param[in] mapInputResults the map considered
         */
        virtual void updateArchiveInputs(std::map<std::reference_wrapper<const Evolution::Individual>, std::shared_ptr<Evaluation::EvaluationResult>> mapInputResults);

        /**
         * \brief Update the output used in the archive based on the individuals given.
         * 
         * Each individual is executed on each input, the output is saved in the archive.
         * 
         * \param[in] individuals the list of individuals evaluated.
         * \param[in] representation the representation used to execute the individuals are evaluated.
         */
        virtual void updateArchiveOutputs(
            const std::vector<std::reference_wrapper<const Evolution::Individual>>& individuals, 
            const Evolution::Representation& representation);
    };
}; // namespace Evaluation

#endif // REINFORCEMENT_AGENT_H
