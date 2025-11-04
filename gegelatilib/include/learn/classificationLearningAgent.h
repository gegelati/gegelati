/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2025) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2022)
 * Pierre-Yves Le Rolland-Raumer <plerolla@insa-rennes.fr> (2020)
 * Quentin Vacher <qvacher@insa-rennes.fr> (2025)
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

#ifndef CLASSIFICATION_LEARNING_AGENT_H
#define CLASSIFICATION_LEARNING_AGENT_H

#include <numeric>
#include <stdexcept>
#include <type_traits>
#include <vector>

#include "data/hash.h"
#include "learn/classificationEvaluationResult.h"
#include "learn/classificationLearningEnvironment.h"
#include "learn/evaluationResult.h"
#include "learn/learningAgent.h"
#include "learn/parallelLearningAgent.h"
#include "selector/classificationSelector.h"

namespace Learn {
    /**
     * \brief LearningAgent specialized for LearningEnvironments representing a
     * classification problem.
     *
     * The key difference between this ClassificationLearningAgent and the base
     * LearningAgent is the way roots are selected for decimation after each
     * generation. In this agent, the roots are decimated based on an average
     * score **per class** instead of decimating roots based on their
     * global average score (over all classes) during the last evaluation.
     * By doing so, the roots providing the best score in each class are
     * preserved which increases the chances of correct classifiers emergence
     * for all classes.
     *
     * In this context, it is assumed that each action of the
     * LearningEnvironment represents a class of the classification problem.
     *
     * The BaseLearningAgent template parameter is the LearningAgent from which
     * the ClassificationLearningAgent inherits. This template notably enable
     * selecting between the classical and the ParallelLearningAgent.
     */
    template <class BaseLearningAgent = ParallelLearningAgent>
    class ClassificationLearningAgent : public BaseLearningAgent
    {
        static_assert(
            std::is_convertible<BaseLearningAgent*, LearningAgent*>::value);

      public:
        /**
         * \brief Constructor for LearningAgent.
         *
         * \param[in] le The LearningEnvironment for the TPG.
         * \param[in] iSet Set of Instruction used to compose Programs in the
         *            learning process.
         * \param[in] p The LearningParameters for the LearningAgent.
         * \param[in] factory The TPGFactory used to create the TPGGraph. A
         * default TPGFactory is used if none is provided.
         */
        ClassificationLearningAgent(
            ClassificationLearningEnvironment& le,
            const Instructions::Set& iSet, const LearningParameters& p,
            const TPG::TPGFactory& factory = TPG::TPGFactory())
            : BaseLearningAgent(le, iSet, p, factory)
        {
            this->selector = std::make_shared<Selector::ClassificationSelector>(
                this->tpg, this->params, le.getNbActions());
        };

        /**
         * \brief Specialization of the evaluateJob method for classification
         * purposes.
         *
         * This method returns a ClassificationEvaluationResult for the
         * evaluated root instead of the usual EvaluationResult. The score per
         * root corresponds to the F1 score for this class.
         */
        virtual std::shared_ptr<EvaluationResult> evaluateJob(
            TPG::TPGExecutionEngine& tee, const Job& root,
            uint64_t generationNumber, LearningMode mode,
            LearningEnvironment& le) const override;
    };

    template <class BaseLearningAgent>
    inline std::shared_ptr<EvaluationResult> ClassificationLearningAgent<
        BaseLearningAgent>::evaluateJob(TPG::TPGExecutionEngine& tee,
                                        const Job& job,
                                        uint64_t generationNumber,
                                        LearningMode mode,
                                        LearningEnvironment& le) const
    {
        // Only consider the first root of jobs as we are not in adversarial
        // mode
        const TPG::TPGVertex* root = job.getRoot();

        // Skip the root evaluation process if enough evaluations were already
        // performed. In the evaluation mode only.
        std::shared_ptr<Learn::EvaluationResult> previousEval;
        if (mode == LearningMode::TRAINING &&
            this->isRootEvalSkipped(*root, previousEval)) {
            return previousEval;
        }

        std::shared_ptr<Selector::SelectionMetrics> selectionMetrics = this->selector->createSelectionMetrics();

        // Evaluate nbIteration times
        for (auto i = 0; i < this->params.nbIterationsPerPolicyEvaluation;
             i++) {
            // Compute a Hash
            Data::Hash<uint64_t> hasher;
            uint64_t hash = hasher(generationNumber) ^ hasher(i);

            // Reset the learning Environment
            le.reset(hash, mode);

            uint64_t nbActions = 0;
            while (!le.isTerminal() &&
                   nbActions < this->params.maxNbActionsPerEval) {
                // Get the actions
                std::vector<double> actionsID =
                    tee.executeFromRoot(*root, le.getInitActions()).second;
                // Do it
                le.doActions(actionsID);
                // Count actions
                nbActions++;
            }

            selectionMetrics->extractMetricsEpisode(root, le);

        }

        (*selectionMetrics) /= this->params.nbIterationsPerPolicyEvaluation;

        // Create the EvaluationResult
        auto evaluationResult = std::shared_ptr<EvaluationResult>(
            new EvaluationResult(selectionMetrics, this->params.nbIterationsPerPolicyEvaluation));

        // Combine it with previous one if any
        if (previousEval != nullptr) {
            *evaluationResult += *previousEval;
        }
        return evaluationResult;
    }
}; // namespace Learn

#endif
