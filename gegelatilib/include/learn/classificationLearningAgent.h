/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2021) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2021)
 * Pierre-Yves Le Rolland-Raumer <plerolla@insa-rennes.fr> (2020)
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

#include "learn/classificationEvaluationResult.h"
#include "learn/classificationLearningEnvironment.h"
#include "learn/evaluationResult.h"
#include "learn/learningAgent.h"
#include "learn/parallelLearningAgent.h"
#include <data/hash.h>

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
         */
        ClassificationLearningAgent(ClassificationLearningEnvironment& le,
                                    const Instructions::Set& iSet,
                                    const LearningParameters& p)
            : BaseLearningAgent(le, iSet, p){};

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

        /**
         * \brief Specialization of the decimateWorstRoots method for
         * classification purposes.
         *
         * During the decimation process, roughly half of the roots are kept
         * based on their score for individual class of the
         * ClassificationLearningEnvironment. To do so, for each class of the
         * ClassificationLearningEnvironment, the roots provided the best score
         * are preserved during the decimation process even if their global
         * score over all classes is not among the best.
         *
         * The remaining half of preserved roots is selected using the general
         * score obtained over all classes.
         *
         * This per-class preservation is activated only if there is a
         * sufficient number of root vertices in the TPGGraph after decimation
         * to guarantee that all classes are preserved equally. In other word,
         * the same number of root is marked for preservation for each class,
         * which can only be achieved if the number of roots to preserve during
         * the decimation process is superior or equal to twice the number of
         * actions of the ClassificationLearningEnvironment. If an insufficient
         * number of root is preserved during the decimation process, all roots
         * are preserved based on their general score.
         *
         * The results map is updated by the method to keep only the results of
         * non-decimated roots.
         */
        void decimateWorstRoots(
            std::multimap<std::shared_ptr<EvaluationResult>,
                          const TPG::TPGVertex*>& results) override;
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

        // Init results
        std::vector<double> result(this->learningEnvironment.getNbActions(),
                                   0.0);
        std::vector<size_t> nbEvalPerClass(
            this->learningEnvironment.getNbActions(), 0);

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
                // Get the action
                uint64_t actionID =
                    ((const TPG::TPGAction*)tee.executeFromRoot(*root).back())
                        ->getActionID();
                // Do it
                le.doAction(actionID);
                // Count actions
                nbActions++;
            }

            // Update results
            const auto& classificationTable =
                ((ClassificationLearningEnvironment&)le)
                    .getClassificationTable();
            // for each class
            for (uint64_t classIdx = 0; classIdx < classificationTable.size();
                 classIdx++) {
                uint64_t truePositive =
                    classificationTable.at(classIdx).at(classIdx);
                uint64_t falseNegative =
                    std::accumulate(classificationTable.at(classIdx).begin(),
                                    classificationTable.at(classIdx).end(),
                                    (uint64_t)0) -
                    truePositive;
                uint64_t falsePositive = 0;
                std::for_each(
                    classificationTable.begin(), classificationTable.end(),
                    [&classIdx, &falsePositive](
                        const std::vector<uint64_t>& classifForClass) {
                        falsePositive += classifForClass.at(classIdx);
                    });
                falsePositive -= truePositive;

                double recall = (double)truePositive /
                                (double)(truePositive + falseNegative);
                double precision = (double)truePositive /
                                   (double)(truePositive + falsePositive);
                // If true positive is 0, set score to 0.
                double fScore = (truePositive != 0) ? 2 * (precision * recall) /
                                                          (precision + recall)
                                                    : 0.0;
                result.at(classIdx) += fScore;

                nbEvalPerClass.at(classIdx) += truePositive + falseNegative;
            }
        }

        // Before returning the EvaluationResult, divide the result per class by
        // the number of iteration
        const LearningParameters& p = this->params;
        std::for_each(result.begin(), result.end(), [p](double& val) {
            val /= (double)p.nbIterationsPerPolicyEvaluation;
        });

        // Create the EvaluationResult
        auto evaluationResult = std::shared_ptr<EvaluationResult>(
            new ClassificationEvaluationResult(result, nbEvalPerClass));

        // Combine it with previous one if any
        if (previousEval != nullptr) {
            *evaluationResult += *previousEval;
        }
        return evaluationResult;
    }

    template <class BaseLearningAgent>
    void ClassificationLearningAgent<BaseLearningAgent>::decimateWorstRoots(
        std::multimap<std::shared_ptr<EvaluationResult>, const TPG::TPGVertex*>&
            results)
    {
        // Check that results are ClassificationEvaluationResults.
        // (also throws on empty results)
        const EvaluationResult* result = results.begin()->first.get();
        if (typeid(ClassificationEvaluationResult) != typeid(*result)) {
            throw std::runtime_error(
                "ClassificationLearningAgent can not decimate worst roots for "
                "results whose type is not ClassificationEvaluationResult.");
        }

        // Compute the number of root to keep/delete base on each criterion
        uint64_t totalNbRoot = this->tpg->getNbRootVertices();
        uint64_t nbRootsToDelete =
            (uint64_t)floor(this->params.ratioDeletedRoots * totalNbRoot);
        uint64_t nbRootsToKeep = (totalNbRoot - nbRootsToDelete);

        // Keep ~half+ of the roots based on their general score on
        // all class.
        // and ~half- of the roots on a per class score (none if nbRoots to keep
        // < 2*nb class)
        uint64_t nbRootsKeptPerClass =
            (nbRootsToKeep / this->learningEnvironment.getNbActions()) / 2;
        uint64_t nbRootsKeptGeneralScore =
            nbRootsToKeep -
            this->learningEnvironment.getNbActions() * nbRootsKeptPerClass;

        // Build a list of roots to keep
        std::vector<const TPG::TPGVertex*> rootsToKeep;

        // Insert roots to keep per class
        for (uint64_t classIdx = 0;
             classIdx < this->learningEnvironment.getNbActions(); classIdx++) {
            // Fill a map with the roots and the score of the specific class as
            // ID.
            std::multimap<double, const TPG::TPGVertex*> sortedRoot;
            std::for_each(
                results.begin(), results.end(),
                [&sortedRoot,
                 &classIdx](const std::pair<std::shared_ptr<EvaluationResult>,
                                            const TPG::TPGVertex*>& res) {
                    sortedRoot.emplace(
                        ((ClassificationEvaluationResult*)res.first.get())
                            ->getScorePerClass()
                            .at(classIdx),
                        res.second);
                });

            // Keep the best nbRootsKeptPerClass (or less for reasons explained
            // in the loop)
            auto iterator = sortedRoot.rbegin();
            for (auto i = 0; i < nbRootsKeptPerClass; i++) {
                // If the root is not already marked to be kept
                if (std::find(rootsToKeep.begin(), rootsToKeep.end(),
                              iterator->second) == rootsToKeep.end()) {
                    rootsToKeep.push_back(iterator->second);
                }
                // Advance the iterator no matter what.
                // This means that if a root scores well for several classes
                // it is kept only once anyway, but additional roots will not
                // be kept for any of the concerned class.
                iterator++;
            }
        }

        // Insert remaining roots to keep
        auto iterator = results.rbegin();
        while (rootsToKeep.size() < nbRootsToKeep &&
               iterator != results.rend()) {
            // If the root is not already marked to be kept
            if (std::find(rootsToKeep.begin(), rootsToKeep.end(),
                          iterator->second) == rootsToKeep.end()) {
                rootsToKeep.push_back(iterator->second);
            }
            // Advance the iterator no matter what.
            iterator++;
        }

        // Do the removal.
        // Because of potential root actions, the preserved number of roots
        // may be higher than the given ratio.
        auto allRoots = this->tpg->getRootVertices();
        auto& tpgRef = this->tpg;
        auto& resultsPerRootRef = this->resultsPerRoot;
        std::for_each(
            allRoots.begin(), allRoots.end(),
            [&rootsToKeep, &tpgRef, &resultsPerRootRef,
             &results](const TPG::TPGVertex* vert) {
                const std::type_info& vertexType = typeid(*vert);
                // Do not remove actions
                if (vertexType != typeid(TPG::TPGAction) &&
                    std::find(rootsToKeep.begin(), rootsToKeep.end(), vert) ==
                        rootsToKeep.end()) {
                    tpgRef->removeVertex(*vert);

                    // Keep only results of non-decimated roots.
                    resultsPerRootRef.erase(vert);

                    // Update results also
                    std::multimap<std::shared_ptr<EvaluationResult>,
                                  const TPG::TPGVertex*>::iterator iter =
                        results.begin();
                    while (iter != results.end()) {
                        if (iter->second == vert) {
                            results.erase(iter);
                            break;
                        }
                        iter++;
                    }
                }
            });
    }
}; // namespace Learn

#endif
