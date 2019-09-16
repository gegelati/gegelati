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

double Learn::LearningAgent::evaluateRoot(const TPG::TPGVertex& root, uint64_t generationNumber, uint64_t nbIterations, uint64_t maxNbActionsPerEval)
{
	// Init results
	double result = 0.0;

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
		result += this->learningEnvironment.getScore() / (double)nbIterations;
	}
	return result;
}

std::multimap<double, const TPG::TPGVertex*> Learn::LearningAgent::evaluateAllRoots(uint64_t generationNumber, uint64_t nbIterations, uint64_t maxNbActionsPerEval)
{
	std::multimap<double, const TPG::TPGVertex*> result;

	for (const TPG::TPGVertex* root : this->tpg.getRootVertices()) {
		double avgScore = this->evaluateRoot(*root, generationNumber, nbIterations, maxNbActionsPerEval);
		result.insert({ avgScore, root });
	}

	return result;
}

void Learn::LearningAgent::trainOneGeneration(double ratioDeletedRoots, uint64_t generationNumber, uint64_t nbIterations, uint64_t maxNbActionsPerEval)
{
	// Populate
	Mutator::TPGMutator::populateTPG(this->tpg, this->archive, this->params);

	// Evaluate
	auto results = this->evaluateAllRoots(generationNumber, nbIterations, maxNbActionsPerEval);

	// Remove worst performing roots
	for (auto i = 0; i < floor(ratioDeletedRoots * params.tpg.nbRoots); i++) {
		tpg.removeVertex(*results.begin()->second);
		results.erase(results.begin());
	}
}

uint64_t Learn::LearningAgent::train(volatile bool& altTraining, bool printProgressBar, double ratioDeletedRoots, uint64_t nbGenerations, uint64_t nbIterations, uint64_t maxNbActionsPerEval)
{
	const int barLength = 50;
	uint64_t generationNumber = 0;

	while (!altTraining && generationNumber < nbGenerations) {
		// Train one generation
		trainOneGeneration(ratioDeletedRoots, generationNumber, nbIterations, maxNbActionsPerEval);
		generationNumber++;

		// Print progressBar (homemade, probably not ideal)
		if (printProgressBar) {
			printf("\rTraining ["); // back
			// filling ratio
			double ratio = (double)generationNumber / (double)nbGenerations;
			int filledPart = (int)((double)ratio * (double)barLength);
			// filled part 
			for (int i = 0; i < filledPart; i++) {
				printf("%c", (char)219);
			}

			// empty part 
			for (int i = filledPart; i < barLength; i++) {
				printf(" ");
			}

			printf("] %4.2f%%", ratio * 100.00);
		}
	}

	if (printProgressBar) {
		if (!altTraining) {
			printf("\nTraining completed\n");
		}
		else {
			printf("\nTraining alted at generation %lld.\n", generationNumber);
		}
	}
	return generationNumber;
}
