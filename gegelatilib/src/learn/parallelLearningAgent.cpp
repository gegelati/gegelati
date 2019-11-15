#include "tpg/tpgExecutionEngine.h"

#include "learn/parallelLearningAgent.h"

double Learn::ParallelLearningAgent::evaluateRoot(const TPG::TPGVertex& root, uint64_t generationNumber, Learn::LearningMode mode, Learn::LearningEnvironment& le, Archive& archive, const Learn::LearningParameters& params)
{
	// Init results
	double result = 0.0;

	// Create the exec engine
	TPG::TPGExecutionEngine tee(&archive);

	// Evaluate nbIteration times
	for (auto i = 0; i < params.nbIterationsPerPolicyEvaluation; i++) {
		// Compute a Hash
		std::hash<uint64_t> hasher;
		uint64_t hash = hasher(generationNumber) ^ hasher(i);

		// Reset the learning Environment
		le.reset(hash, mode);

		uint64_t nbActions = 0;
		while (!le.isTerminal() && nbActions < params.maxNbActionsPerEval) {
			// Get the action
			uint64_t actionID = ((const TPG::TPGAction*)tee.executeFromRoot(root).back())->getActionID();
			// Do it
			le.doAction(actionID);
			// Count actions
			nbActions++;
		}

		// Update results
		result += le.getScore();
	}
	return result / (double)params.nbIterationsPerPolicyEvaluation;
}

std::multimap<double, const TPG::TPGVertex*> Learn::ParallelLearningAgent::evaluateAllRoots(uint64_t generationNumber, Learn::LearningMode mode)
{
	std::multimap<double, const TPG::TPGVertex*> result;

	if (this->maxNbThreads <= 1 || !this->learningEnvironment.isCopyable()) {
		// Sequential mode
		for (const TPG::TPGVertex* root : this->tpg.getRootVertices()) {
			double avgScore = ParallelLearningAgent::evaluateRoot(*root, generationNumber, mode, this->learningEnvironment, this->archive, this->params);
			result.insert({ avgScore, root });
		}
	}
	else {
		// Parallel mode
	}

	return result;
}