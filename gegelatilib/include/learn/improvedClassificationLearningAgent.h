#ifndef GEGELATI_IMPROVEDCLASSIFICATIONLEARNINGAGENT_H
#define GEGELATI_IMPROVEDCLASSIFICATIONLEARNINGAGENT_H

#include <ctime>

#include "learn/classificationEvaluationResult.h"
#include "learn/improvedClassificationLearningEnvironment.h"
#include "learn/learningAgent.h"
#include "learn/parallelLearningAgent.h"

namespace Learn
{
    typedef enum LearningAlgorithm
    {
        DEFAULT, BRSS, FS, BANDIT, LEXICASE
    }LearningAlgorithm;

    //--------------------------------------------------------------------------------------------------------------------------------------------------------

    template <class BaseLearningAgent = ParallelLearningAgent>
    class ImprovedClassificationLearningAgent : public BaseLearningAgent
    {
        static_assert(std::is_convertible<BaseLearningAgent*, LearningAgent*>::value);

      protected:
        LearningAlgorithm _type;
        float _datsubsetSizeRatio;

        /// All the 'evaluateJob functions', one for each learning algorithm
        std::shared_ptr<EvaluationResult> evaluateJob_BRSS(TPG::TPGExecutionEngine& tee, const Job& job, uint64_t generationNumber, LearningMode mode, Learn::ImprovedClassificationLearningEnvironment& le) const;
        std::shared_ptr<EvaluationResult> evaluateJob_FS(TPG::TPGExecutionEngine& tee, const Job& job, uint64_t generationNumber, LearningMode mode, Learn::ImprovedClassificationLearningEnvironment& le) const;
        std::shared_ptr<EvaluationResult> evaluateJob_BANDIT(TPG::TPGExecutionEngine& tee, const Job& job, uint64_t generationNumber, LearningMode mode, Learn::ImprovedClassificationLearningEnvironment& le) const;
        std::shared_ptr<EvaluationResult> evaluateJob_LEXICASE(TPG::TPGExecutionEngine& tee, const Job& job, uint64_t generationNumber, LearningMode mode, Learn::ImprovedClassificationLearningEnvironment& le) const;
        std::shared_ptr<EvaluationResult> evaluateJob_DEFAULT(TPG::TPGExecutionEngine& tee, const Job& job, uint64_t generationNumber, LearningMode mode, Learn::ImprovedClassificationLearningEnvironment& le) const;

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
         * \param[in] type The LearningAlgorithm that would be used by the agent
         */
        ImprovedClassificationLearningAgent<BaseLearningAgent>(Learn::ImprovedClassificationLearningEnvironment& le, const Instructions::Set& iSet, const LearningParameters& p, const TPG::TPGFactory& factory = TPG::TPGFactory(), LearningAlgorithm type = DEFAULT)
            : BaseLearningAgent(le, iSet, p, factory), _type(type), _datsubsetSizeRatio(0.6)
                                                                        {};

        /**
         * \brief Specialization of the evaluateJob method for classification
         * purposes.
         *
         * This method returns a ClassificationEvaluationResult for the
         * evaluated root instead of the usual EvaluationResult. The score per
         * root corresponds to the F1 score for this class.
         */
        std::shared_ptr<EvaluationResult> evaluateJob(TPG::TPGExecutionEngine& tee, const Job& job, uint64_t generationNumber, LearningMode mode, Learn::LearningEnvironment& le) const override;

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
        void decimateWorstRoots(std::multimap<std::shared_ptr<EvaluationResult>, const TPG::TPGVertex*>& results) override;
    };

    //--------------------------------------------------------------------------------------------------------------------------------------------------------

    template <class BaseLearningAgent>
    inline std::shared_ptr<EvaluationResult> ImprovedClassificationLearningAgent<BaseLearningAgent>::evaluateJob(TPG::TPGExecutionEngine& tee, const Job& job, uint64_t generationNumber, LearningMode mode, Learn::LearningEnvironment& le) const
    {
        auto icle = dynamic_cast<Learn::ImprovedClassificationLearningEnvironment&>((Learn::LearningEnvironment&)le);

        switch(this->_type)
        {
        case (DEFAULT):
            return evaluateJob_DEFAULT(tee, job, generationNumber, mode, icle);
        case (BRSS):
            return evaluateJob_BRSS(tee, job, generationNumber, mode, icle);
        case (FS):
            return evaluateJob_FS(tee, job, generationNumber, mode, icle);
        case (BANDIT):
            return evaluateJob_BANDIT(tee, job, generationNumber, mode, icle);
        case (LEXICASE):
            return evaluateJob_LEXICASE(tee, job, generationNumber, mode, icle);
        default:
            break;
        }
    }

    template <class BaseLearningAgent>
    inline std::shared_ptr<EvaluationResult> ImprovedClassificationLearningAgent<BaseLearningAgent>::evaluateJob_DEFAULT(TPG::TPGExecutionEngine& tee, const Job& job, uint64_t generationNumber, LearningMode mode, Learn::ImprovedClassificationLearningEnvironment& le) const
    {
        // Only consider the first root of jobs as we are not in adversarial
        // mode
        auto root = job.getRoot();

        // Skip the root evaluation process if enough evaluations were already
        // performed. In the evaluation mode only.
        std::shared_ptr<Learn::EvaluationResult> previousEval;
        if (mode == LearningMode::TRAINING && this->isRootEvalSkipped(*root, previousEval))
            return previousEval;

        // Init results
        std::vector<double> result(this->learningEnvironment.getNbActions(), 0.0);
        std::vector<size_t> nbEvalPerClass(this->learningEnvironment.getNbActions(), 0);

        // Evaluate nbIteration times
        for (auto i = 0; i < this->params.nbIterationsPerPolicyEvaluation; i++)
        {
            // Compute a Hash
            Data::Hash<uint64_t> hasher;
            uint64_t hash = hasher(generationNumber) ^ hasher(i);

            // Reset the learning Environment
            le.reset(hash, mode);

            uint64_t nbActions = 0;
            while (!le.isTerminal() && nbActions < this->params.maxNbActionsPerEval)
            {
                // Get the action
                uint64_t actionID = ((const TPG::TPGAction*)tee.executeFromRoot(*root).back())->getActionID();
                // Do it
                le.doAction(actionID);
                // Count actions
                nbActions++;
            }

            // Update results
            const auto& classificationTable = le.getClassificationTable();
            // for each class
            for (uint64_t classIdx = 0; classIdx < classificationTable.size(); classIdx++)
            {
                uint64_t truePositive = classificationTable.at(classIdx).at(classIdx);
                uint64_t falseNegative = std::accumulate(classificationTable.at(classIdx).begin(), classificationTable.at(classIdx).end(), (uint64_t)0) - truePositive;
                uint64_t falsePositive = 0;
                std::for_each(classificationTable.begin(), classificationTable.end(), [&classIdx, &falsePositive](const std::vector<uint64_t>& classifForClass)
                              {
                                  falsePositive += classifForClass.at(classIdx);
                              });
                falsePositive -= truePositive;

                double recall = (double)truePositive / (double)(truePositive + falseNegative);
                double precision = (double)truePositive / (double)(truePositive + falsePositive);
                // If true positive is 0, set score to 0.
                double fScore = (truePositive != 0) ? 2 * (precision * recall) / (precision + recall) : 0.0;
                result.at(classIdx) += fScore;

                nbEvalPerClass.at(classIdx) += truePositive + falseNegative;
            }
        }

        // Before returning the EvaluationResult, divide the result per class by
        // the number of iteration
        const LearningParameters& p = this->params;
        std::for_each(result.begin(), result.end(), [p](double& val)
                      {
                          val /= (double)p.nbIterationsPerPolicyEvaluation;
                      });

        // Create the EvaluationResult
        auto evaluationResult = std::shared_ptr<EvaluationResult>(new ClassificationEvaluationResult(result, nbEvalPerClass));

        // Combine it with previous one if any
        if (previousEval != nullptr)
            *evaluationResult += *previousEval;
        return evaluationResult;
    }

    template <class BaseLearningAgent>
    inline std::shared_ptr<EvaluationResult> ImprovedClassificationLearningAgent<BaseLearningAgent>::evaluateJob_BRSS(TPG::TPGExecutionEngine& tee, const Job& job, uint64_t generationNumber, LearningMode mode, Learn::ImprovedClassificationLearningEnvironment& le) const
    {
        // Only consider the first root of jobs as we are not in adversarial
        // mode
        auto root = job.getRoot();

        // Skip the root evaluation process if enough evaluations were already
        // performed. In the evaluation mode only.
        std::shared_ptr<Learn::EvaluationResult> previousEval;
        if (mode == LearningMode::TRAINING && this->isRootEvalSkipped(*root, previousEval))
            return previousEval;

        // Init results
        std::vector<double> result(this->learningEnvironment.getNbActions(), 0.0);
        std::vector<size_t> nbEvalPerClass(this->learningEnvironment.getNbActions(), 0);

        // Init the datasubset
        auto datasubset = new DS();

        for(int i=0 ; i<this->_datsubsetSizeRatio * le._dataset->first.size() ; i++)
        {
            for(int j=0 ; j<le._dataset->first.at(i).size() ; j++)
                datasubset->first.at(i).push_back(le._dataset->first.at(i).at(j));
            datasubset->second.push_back(le._dataset->second.at(i));
        }

        //first refresh the datasubset
        le.refreshDatasubset((this->_type == BANDIT || this->_type == LEXICASE) + 2 * (this->_type == DEFAULT), le._rng.getUnsignedInt64(0, datasubset->first.size()-1));

        // Evaluate nbIteration times
        for (auto i = 0; i < this->params.nbIterationsPerPolicyEvaluation; i++)
        {
            // Compute a Hash
            Data::Hash<uint64_t> hasher;
            uint64_t hash = hasher(generationNumber) ^ hasher(i);

            // Reset the learning Environment
            le.reset(hash, mode);

            uint64_t nbActions = 0;
            while (!le.isTerminal() && nbActions < this->params.maxNbActionsPerEval)
            {
                // Get the action
                uint64_t actionID = ((const TPG::TPGAction*)tee.executeFromRoot(*root).back())->getActionID();
                // Do it
                le.doAction(actionID);
                // Count actions
                nbActions++;
            }

            // Update results
            const auto& classificationTable = ((ImprovedClassificationLearningEnvironment&)le).getClassificationTable();
            // for each class
            for (uint64_t classIdx = 0; classIdx < classificationTable.size(); classIdx++)
            {
                double n=0;
                for(auto id=0 ; id < classificationTable.size() ; id++)
                    n+= (double)classificationTable.at(classIdx).at(id);
                result.at(classIdx) = (double)classificationTable.at(classIdx).at(classIdx) / n;
            }

            le.refreshDatasubset((this->_type == BANDIT || this->_type == LEXICASE) + 2 * (this->_type == DEFAULT), le._rng.getUnsignedInt64(0, datasubset->first.size()-1));
        }

        // Before returning the EvaluationResult, divide the result per class by
        // the number of iteration
        const LearningParameters& p = this->params;
        std::for_each(result.begin(), result.end(), [p](double& val)
                      {
                          val /= (double)p.nbIterationsPerPolicyEvaluation;
                      });

        // Create the EvaluationResult
        auto evaluationResult = std::shared_ptr<EvaluationResult>(new ClassificationEvaluationResult(result, nbEvalPerClass));

        // Combine it with previous one if any
        if (previousEval != nullptr)
            *evaluationResult += *previousEval;

        return evaluationResult;
    }

    template <class BaseLearningAgent>
    inline std::shared_ptr<EvaluationResult> ImprovedClassificationLearningAgent<BaseLearningAgent>::evaluateJob_FS(TPG::TPGExecutionEngine& tee, const Job& job, uint64_t generationNumber, LearningMode mode, Learn::ImprovedClassificationLearningEnvironment& le) const
    {
        return nullptr;
    }

    template <class BaseLearningAgent>
    inline std::shared_ptr<EvaluationResult> ImprovedClassificationLearningAgent<BaseLearningAgent>::evaluateJob_BANDIT(TPG::TPGExecutionEngine& tee, const Job& job, uint64_t generationNumber, LearningMode mode, Learn::ImprovedClassificationLearningEnvironment& le) const
    {
        return nullptr;
    }

    template <class BaseLearningAgent>
    inline std::shared_ptr<EvaluationResult> ImprovedClassificationLearningAgent<BaseLearningAgent>::evaluateJob_LEXICASE(TPG::TPGExecutionEngine& tee, const Job& job, uint64_t generationNumber, LearningMode mode, Learn::ImprovedClassificationLearningEnvironment& le)const
    {
        return nullptr;
    }




    template <class BaseLearningAgent>
    void ImprovedClassificationLearningAgent<BaseLearningAgent>::decimateWorstRoots(std::multimap<std::shared_ptr<EvaluationResult>, const TPG::TPGVertex*>& results)
    {
        // Check that results are ClassificationEvaluationResults.
        // (also throws on empty results)
        if (typeid(ClassificationEvaluationResult) != typeid(results.begin()->first.get()))
            throw std::runtime_error("ClassificationLearningAgent can not decimate worst roots for " "results whose type is not ClassificationEvaluationResult.");

        // Compute the number of root to keep/delete base on each criterion
        uint64_t totalNbRoot = this->tpg->getNbRootVertices();
        auto nbRootsToDelete = (uint64_t)floor(this->params.ratioDeletedRoots * totalNbRoot);
        uint64_t nbRootsToKeep = (totalNbRoot - nbRootsToDelete);

        // Keep ~half+ of the roots based on their general score on
        // all class.
        // and ~half- of the roots on a per class score (none if nbRoots to keep
        // < 2*nb class)
        uint64_t nbRootsKeptPerClass = (nbRootsToKeep / this->learningEnvironment.getNbActions()) / 2;
        //uint64_t nbRootsKeptGeneralScore = nbRootsToKeep - this->learningEnvironment.getNbActions() * nbRootsKeptPerClass;

        // Build a list of roots to keep
        std::vector<const TPG::TPGVertex*> rootsToKeep;

        // Insert roots to keep per class
        for (uint64_t classIdx = 0; classIdx < this->learningEnvironment.getNbActions(); classIdx++)
        {
            // Fill a map with the roots and the score of the specific class as
            // ID.
            std::multimap<double, const TPG::TPGVertex*> sortedRoot;
            std::for_each(results.begin(), results.end(), [&sortedRoot, &classIdx](const std::pair<std::shared_ptr<EvaluationResult>, const TPG::TPGVertex*>& res)
                          {
                              sortedRoot.emplace(((ClassificationEvaluationResult*)res.first.get())->getScorePerClass().at(classIdx), res.second);
                          });

            // Keep the best nbRootsKeptPerClass (or less for reasons explained
            // in the loop)
            auto iterator = sortedRoot.rbegin();
            for (auto i = 0; i < nbRootsKeptPerClass; i++)
            {
                // If the root is not already marked to be kept
                if (std::find(rootsToKeep.begin(), rootsToKeep.end(), iterator->second) == rootsToKeep.end())
                    rootsToKeep.push_back(iterator->second);
                // Advance the iterator no matter what.
                // This means that if a root scores well for several classes
                // it is kept only once anyway, but additional roots will not
                // be kept for any of the concerned class.
                iterator++;
            }
        }

        // Insert remaining roots to keep
        auto iterator = results.rbegin();
        while (rootsToKeep.size() < nbRootsToKeep && iterator != results.rend())
        {
            // If the root is not already marked to be kept
            if (std::find(rootsToKeep.begin(), rootsToKeep.end(), iterator->second) == rootsToKeep.end())
                rootsToKeep.push_back(iterator->second);
            // Advance the iterator no matter what.
            iterator++;
        }

        // Do the removal.
        // Because of potential root actions, the preserved number of roots
        // may be higher than the given ratio.
        auto allRoots = this->tpg->getRootVertices();
        auto& tpgRef = this->tpg;
        auto& resultsPerRootRef = this->resultsPerRoot;
        std::for_each(allRoots.begin(), allRoots.end(), [&rootsToKeep, &tpgRef, &resultsPerRootRef, &results](const TPG::TPGVertex* vert)
                      {
                          // Do not remove actions
                          if (dynamic_cast<const TPG::TPGAction*>(vert) == nullptr && std::find(rootsToKeep.begin(), rootsToKeep.end(), vert) == rootsToKeep.end())
                          {
                              tpgRef->removeVertex(*vert);

                              // Keep only results of non-decimated roots.
                              resultsPerRootRef.erase(vert);

                              // Update results also
                              auto iter = results.begin();
                              while (iter != results.end())
                              {
                                  if (iter->second == vert)
                                  {
                                      results.erase(iter);
                                      break;
                                  }
                                  iter++;
                              }
                          }
                      });
    }
}

#endif // GEGELATI_IMPROVEDCLASSIFICATIONLEARNINGAGENT_H
