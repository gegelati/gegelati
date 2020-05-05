/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2020) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2020)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2019 - 2020)
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

#include <inttypes.h>

#include "data/hash.h"
#include "tpg/tpgExecutionEngine.h"
#include "mutator/rng.h"
#include "mutator/tpgMutator.h"
#include "learn/evaluationResult.h"

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

std::shared_ptr<Learn::EvaluationResult> Learn::LearningAgent::evaluateRoot(TPG::TPGExecutionEngine& tee, const TPG::TPGVertex& root, uint64_t generationNumber, Learn::LearningMode mode, LearningEnvironment& le) const
{
	// Init results
	double result = 0.0;

	// Evaluate nbIteration times
	for (auto i = 0; i < this->params.nbIterationsPerPolicyEvaluation; i++) {
		// Compute a Hash
		Data::Hash<uint64_t> hasher;
		uint64_t hash = hasher(generationNumber) ^ hasher(i);

		// Reset the learning Environment
		le.reset(hash, mode);

		uint64_t nbActions = 0;
		while (!le.isTerminal() && nbActions < this->params.maxNbActionsPerEval) {
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
	return std::shared_ptr<EvaluationResult>(new EvaluationResult(result / (double)params.nbIterationsPerPolicyEvaluation));
}

std::multimap< std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex*> Learn::LearningAgent::evaluateAllRoots(uint64_t generationNumber, Learn::LearningMode mode)
{
	std::multimap< std::shared_ptr<EvaluationResult>, const TPG::TPGVertex*> result;

	// Create the TPGExecutionEngine for this evaluation.
	// The engine uses the Archive only in training mode.
	TPG::TPGExecutionEngine tee(this->env, (mode == LearningMode::TRAINING) ? &this->archive : NULL);

	for (const TPG::TPGVertex* root : this->tpg.getRootVertices()) {
		// Before each root evaluation, set a new seed for the archive in TRAINING Mode
		// Else, archiving should be deactivate anyway
		if (mode == LearningMode::TRAINING) {
			this->archive.setRandomSeed(this->rng.getUnsignedInt64(0, UINT64_MAX));
		}

		std::shared_ptr<EvaluationResult> avgScore = this->evaluateRoot(tee, *root, generationNumber, mode, this->learningEnvironment);
		result.emplace(avgScore, root); //{ avgScore, root });
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

void Learn::LearningAgent::decimateWorstRoots(std::multimap<std::shared_ptr<EvaluationResult>, const TPG::TPGVertex*>& results)
{
	auto i = 0;
	while (i < floor(this->params.ratioDeletedRoots * params.mutation.tpg.nbRoots)
		&& results.size() > 0) {
		// If the root is an action, do not remove it!
		if (typeid(*results.begin()->second) != typeid(TPG::TPGAction)) {
			tpg.removeVertex(*results.begin()->second);
		}
		else {
			i--; // no vertex was actually removed
		}
		results.erase(results.begin());

		// Increment loop counter
		i++;
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
