#ifndef CLASSIFICATION_LEARNING_AGENT_H
#define CLASSIFICATION_LEARNING_AGENT_H

#include <type_traits>
#include <vector>
#include <numeric>
#include <stdexcept>

#include "learn/evaluationResult.h"
#include "learn/classificationEvaluationResult.h"
#include "learn/learningAgent.h"
#include "learn/parallelLearningAgent.h"
#include "learn/classificationLearningEnvironment.h"

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
	template <class BaseLearningAgent = ParallelLearningAgent>  class ClassificationLearningAgent : public BaseLearningAgent {
		static_assert(std::is_convertible<BaseLearningAgent*, LearningAgent*>::value);

	public:
		/**
		* \brief Constructor for LearningAgent.
		*
		* \param[in] le The LearningEnvironment for the TPG.
		* \param[in] iSet Set of Instruction used to compose Programs in the
		*            learning process.
		* \param[in] p The LearningParameters for the LearningAgent.
		* \param[in] nbRegs The number of registers for the execution
		*                   environment of Program.
		*/
		ClassificationLearningAgent(ClassificationLearningEnvironment& le, const Instructions::Set& iSet, const LearningParameters& p, const unsigned int nbRegs = 8) : BaseLearningAgent(le, iSet, p, nbRegs) {};

		/// Specialization for classificationPurposes
		std::shared_ptr<EvaluationResult> evaluateRoot(TPG::TPGExecutionEngine& tee, const TPG::TPGVertex& root, uint64_t generationNumber, LearningMode mode) override;

		/**
		* \brief Decimate worst root specialized for classification purposes.
		*/
		void decimateWorstRoots(std::multimap<std::shared_ptr<EvaluationResult>, const TPG::TPGVertex*>& results) override;
	};

	template<class BaseLearningAgent>
	inline std::shared_ptr<EvaluationResult> ClassificationLearningAgent<BaseLearningAgent>::evaluateRoot(TPG::TPGExecutionEngine& tee, const TPG::TPGVertex& root, uint64_t generationNumber, LearningMode mode)
	{
		// Init results
		std::vector<double> result(this->learningEnvironment.getNbActions(), 0.0);

		// Evaluate nbIteration times
		for (auto i = 0; i < this->params.nbIterationsPerPolicyEvaluation; i++) {
			// Compute a Hash
			std::hash<uint64_t> hasher;
			uint64_t hash = hasher(generationNumber) ^ hasher(i);

			// Reset the learning Environment
			this->learningEnvironment.reset(hash, mode);

			uint64_t nbActions = 0;
			while (!this->learningEnvironment.isTerminal() && nbActions < this->params.maxNbActionsPerEval) {
				// Get the action
				uint64_t actionID = ((const TPG::TPGAction*)tee.executeFromRoot(root).back())->getActionID();
				// Do it
				this->learningEnvironment.doAction(actionID);
				// Count actions
				nbActions++;
			}

			// Update results
			const auto& classificationTable = ((ClassificationLearningEnvironment&)this->learningEnvironment).getClassificationTable();
			// for each class
			for (uint64_t classIdx = 0; classIdx < classificationTable.size(); classIdx++) {
				// result for the class is the number of total guess for this 
				// class divided by the total number of time this class was 
				// presented to the LearningAgent
				result.at(classIdx) += (double)classificationTable.at(classIdx).at(classIdx) / (double)std::accumulate(classificationTable.at(classIdx).begin(), classificationTable.at(classIdx).end(), 0.0);
			}
		}

		// Before returning the EvaluationResult, divide the result per class by the number of iteration
		const LearningParameters& p = this->params;
		std::for_each(result.begin(), result.end(), [p](double& val) { val /= (double)p.nbIterationsPerPolicyEvaluation; });

		return std::shared_ptr<EvaluationResult>(new ClassificationEvaluationResult(result));
	}

	template<class BaseLearningAgent>
	void ClassificationLearningAgent<BaseLearningAgent>::decimateWorstRoots(std::multimap <std::shared_ptr<EvaluationResult>, const TPG::TPGVertex* >& results) {
		// Check that results are ClassificationEvaluationResults.
		// (also throws on empty results)
		if (typeid(ClassificationEvaluationResult) != typeid(*(results.begin()->first.get()))) {
			throw std::runtime_error("ClassificationLearningAgent can not decimate worst roots for results whose type is not ClassificationEvaluationResult.");
		}

		// Compute the number of root to keep/delete base on each criterion
		uint64_t totalNbRoot = this->tpg.getNbRootVertices();
		uint64_t nbRootsToDelete = (uint64_t)floor(this->params.ratioDeletedRoots * totalNbRoot);
		uint64_t nbRootsToKeep = (totalNbRoot - nbRootsToDelete);

		// Keep ~half+ of the roots based on their general score on 
		// all class.
		// and ~half- of the roots on a per class score (none if nbRoots to keep < 2*nb class)
		uint64_t nbRootsKeptPerClass = (nbRootsToKeep / this->learningEnvironment.getNbActions()) / 2;
		uint64_t nbRootsKeptGeneralScore = nbRootsToKeep - this->learningEnvironment.getNbActions() * nbRootsKeptPerClass;

		// Build a list of roots to keep
		std::vector<const TPG::TPGVertex*> rootsToKeep;

		// Insert roots to keep per class
		for (uint64_t classIdx = 0; classIdx < this->learningEnvironment.getNbActions(); classIdx++) {
			// Fill a map with the roots and the score of the specific class as ID.
			std::multimap<double, const TPG::TPGVertex*> sortedRoot;
			std::for_each(results.begin(), results.end(), [&sortedRoot, &classIdx](const std::pair<std::shared_ptr<EvaluationResult>, const TPG::TPGVertex* >& res)
				{
					sortedRoot.emplace(((ClassificationEvaluationResult*)res.first.get())->getScorePerClass().at(classIdx), res.second);
				});

			// Keep the best nbRootsKeptPerClass (or less for reasons explained in the loop)
			auto iterator = sortedRoot.rbegin();
			for (auto i = 0; i < nbRootsKeptPerClass; i++) {
				// If the root is not already marked to be kept
				if (std::find(rootsToKeep.begin(), rootsToKeep.end(), iterator->second) == rootsToKeep.end()) {
					rootsToKeep.push_back(iterator->second);
				}
				// Advance the iterator no matter what.
				// This means that is a root scores well for several classes
				// it is kept only once anyway, but additional roots will not 
				// be kept for any of the concerned class.
				iterator++;
			}
		}

		// Insert remaining roots to keep
		auto iterator = results.rbegin();
		while (rootsToKeep.size() < nbRootsToKeep) {
			// If the root is not already marked to be kept
			if (std::find(rootsToKeep.begin(), rootsToKeep.end(), iterator->second) == rootsToKeep.end()) {
				rootsToKeep.push_back(iterator->second);
			}
			// Advance the iterator no matter what.
			iterator++;
		}

		// Do the removal.
		// Because of potential root actions, the remaining number of roots
		// may be higher than the given ratio.
		auto allRoots = this->tpg.getRootVertices();
		auto& tpgRef = this->tpg;
		std::for_each(allRoots.begin(), allRoots.end(), [&rootsToKeep, &tpgRef](const TPG::TPGVertex* vert)
			{
				if (std::find(rootsToKeep.begin(), rootsToKeep.end(), vert) == rootsToKeep.end()) {
					tpgRef.removeVertex(*vert);
				}
			});
	}
};

#endif