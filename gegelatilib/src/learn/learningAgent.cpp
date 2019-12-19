#include <inttypes.h>

#include "tpg/tpgExecutionEngine.h"
#include "mutator/rng.h"
#include "mutator/tpgMutator.h"

#include "learn/learningAgent.h"

TPG::TPGGraph& Learn::LearningAgent::getTPGGraph()
{
	return this->tpg;
}

const Archive& Learn::LearningAgent::getArchive() const
{
	return this->archive;
}

Mutator::RNG& Learn::LearningAgent::getRNG() {
	return this->rng;
}

void Learn::LearningAgent::init(uint64_t seed) {
	// Initialize Randomness
	this->rng.setSeed(seed);

	// Initialize the tpg
	Mutator::TPGMutator::initRandomTPG(this->tpg, params.mutation, this->rng);

	// Clear the archive
	this->archive.clear();
}

double Learn::LearningAgent::evaluateRoot(TPG::TPGExecutionEngine& tee, const TPG::TPGVertex& root, uint64_t generationNumber, Learn::LearningMode mode)
{
	// Init results
	double result = 0.0;

	// Evaluate nbIteration times
	for (auto i = 0; i < params.nbIterationsPerPolicyEvaluation; i++) {
		// Compute a Hash
		std::hash<uint64_t> hasher;
		uint64_t hash = hasher(generationNumber) ^ hasher(i);

		// Reset the learning Environment
		this->learningEnvironment.reset(hash, mode);

		uint64_t nbActions = 0;
		while (!this->learningEnvironment.isTerminal() && nbActions < params.maxNbActionsPerEval) {
			// Get the action
			uint64_t actionID = ((const TPG::TPGAction*)tee.executeFromRoot(root).back())->getActionID();
			// Do it
			this->learningEnvironment.doAction(actionID);
			// Count actions
			nbActions++;
		}

		// Update results
		result += this->learningEnvironment.getScore();
	}
	return result / (double)params.nbIterationsPerPolicyEvaluation;
}

std::multimap<double, const TPG::TPGVertex*> Learn::LearningAgent::evaluateAllRoots(uint64_t generationNumber, Learn::LearningMode mode)
{
	std::multimap<double, const TPG::TPGVertex*> result;

	// Create the TPGExecutionEngine for this evaluation.
	// The engine uses the Archive only in training mode.
	TPG::TPGExecutionEngine tee(this->env, (mode == LearningMode::TRAINING) ? &this->archive : NULL);

	for (const TPG::TPGVertex* root : this->tpg.getRootVertices()) {
		// Before each root evaluation, set a new seed for the archive in TRAINING Mode
		// Else, archiving should be deactivate anyway
		if (mode == LearningMode::TRAINING) {
			this->archive.setRandomSeed(this->rng.getUnsignedInt64(0, UINT64_MAX));
		}

		double avgScore = this->evaluateRoot(tee, *root, generationNumber, mode);
		result.insert({ avgScore, root });
	}

	return result;
}

void Learn::LearningAgent::trainOneGeneration(uint64_t generationNumber)
{
	// Populate Sequentially
	Mutator::TPGMutator::populateTPG(this->tpg, this->archive, this->params.mutation, this->rng, 1);

	// Evaluate
	auto results = this->evaluateAllRoots(generationNumber, LearningMode::TRAINING);
	// Remove worst performing roots
	decimateWorstRoots(results);
}

void Learn::LearningAgent::decimateWorstRoots(std::multimap<double, const TPG::TPGVertex*>& results)
{
	for (auto i = 0; i < floor(this->params.ratioDeletedRoots * params.mutation.tpg.nbRoots); i++) {
		// If the root is an action, do not remove it!
		if (typeid(*results.begin()->second) != typeid(TPG::TPGAction)) {

			tpg.removeVertex(*results.begin()->second);
		}
		else {
			i--; // no vertex was actually removed
		}
		results.erase(results.begin());
	}
}

uint64_t Learn::LearningAgent::train(volatile bool& altTraining, bool printProgressBar)
{
	const int barLength = 50;
	uint64_t generationNumber = 0;

	while (!altTraining && generationNumber < this->params.nbGenerations) {
		// Train one generation
		trainOneGeneration(generationNumber);
		generationNumber++;

		// Print progressBar (homemade, probably not ideal)
		if (printProgressBar) {
			printf("\rTraining ["); // back
			// filling ratio
			double ratio = (double)generationNumber / (double)this->params.nbGenerations;
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
			printf("\nTraining alted at generation %" PRIu64 ".\n", generationNumber);
		}
	}
	return generationNumber;
}

void Learn::LearningAgent::keepBestPolicy()
{
	// Evaluate all roots
	auto results = this->evaluateAllRoots(0, LearningMode::VALIDATION);
	auto iterResults = results.begin();
	std::advance(iterResults, results.size() - 1);
	auto bestRoot = iterResults->second;

	// Remove all but the best root from the tpg
	while (this->tpg.getNbRootVertices() != 1) {
		auto roots = this->tpg.getRootVertices();
		for (auto root : roots) {
			if (root != bestRoot) {
				tpg.removeVertex(*root);
			}
		}
	}
}
