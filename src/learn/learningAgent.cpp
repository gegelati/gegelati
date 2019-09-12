#include "tpg/tpgExecutionEngine.h"
#include "mutator/rng.h"
#include "mutator/tpgMutator.h"

#include "learn/learningAgent.h"

TPG::TPGGraph& Learn::LearningAgent::getTPGGraph()
{
	return this->tpg;
}

void Learn::LearningAgent::init(uint64_t seed) {
	// Initialize Randomness
	Mutator::RNG::setSeed(seed);

	// Initialize the tpg
	Mutator::TPGMutator::initRandomTPG(this->tpg, params);

	// Clear the archive
	this->archive.clear();
}

std::pair<double, double> Learn::LearningAgent::evaluateRoot(const TPG::TPGVertex& root, uint64_t generationNumber, uint64_t nbIterations, uint64_t maxNbActionsPerEval)
{
	// Init results
	std::pair<double, double> result = { 0.0,0.0 };

	// Create the exec engine
	TPG::TPGExecutionEngine tee(&this->archive);

	// Evaluate nbIteration times
	for (auto i = 0; i < nbIterations; i++) {
		// Compute a Hash
		std::hash<uint64_t> hasher;
		uint64_t hash = hasher(generationNumber) ^ hasher(i);

		// Reset the learning Environment
		this->learningEnvironment.reset(hash);

		uint64_t nbActions = 0;
		while (!this->learningEnvironment.isTerminal() && nbActions < maxNbActionsPerEval) {
			// Get the action
			uint64_t actionID = ((const TPG::TPGAction*)tee.executeFromRoot(root).back())->getActionID();
			// Do it
			this->learningEnvironment.doAction(actionID);
			// Count actions
			nbActions++;
		}

		// Update results
		result.first += this->learningEnvironment.getScore() / (double)nbIterations;
		result.second += (double)nbActions / (double)nbIterations;
	}
	return result;
}
