#include <algorithm>
#include <iterator>
#include <thread>
#include <queue>
#include <mutex>

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
	std::multimap<double, const TPG::TPGVertex*> results;

	if (this->maxNbThreads <= 1 || !this->learningEnvironment.isCopyable()) {
		// Sequential mode
		for (const TPG::TPGVertex* root : this->tpg.getRootVertices()) {
			double avgScore = ParallelLearningAgent::evaluateRoot(*root, generationNumber, mode, this->learningEnvironment, this->archive, this->params);
			results.insert({ avgScore, root });
		}
	}
	else {
		// Parallel mode
		evaluateAllRootsInParallel(generationNumber, mode, results);
	}

	return results;
}

void Learn::ParallelLearningAgent::slaveEvalRootThread(uint64_t generationNumber, LearningMode mode,
	std::queue<std::pair<uint64_t, const TPG::TPGVertex*>>& rootsToProcess, std::mutex& rootsToProcessMutex,
	std::multimap<double, const TPG::TPGVertex*>& results, std::mutex& resultsMutex,
	std::map<uint64_t, ExhaustiveArchive*>& archiveMap, std::mutex& archiveMapMutex,
	uint64_t& nextArchiveToMerge, std::mutex& archiveMergingMutex) {

	// Clone learningEnvironment
	LearningEnvironment* privateLearningEnvironment = this->learningEnvironment.clone();

	// Pop a job
	while (!rootsToProcess.empty()) { // Thread safe access to size
		bool doProcess = false;
		std::pair<uint64_t, const TPG::TPGVertex*> rootToProcess;
		{ // Mutuel exclusion zone
			std::lock_guard<std::mutex> lock(rootsToProcessMutex);
			if (!rootsToProcess.empty()) { // Additional verification after lock
				rootToProcess = rootsToProcess.front();
				rootsToProcess.pop();
				doProcess = true;
			}
		} // End of mutual exclusion zone

		// Processing to do?
		if (doProcess) {
			doProcess = false;
			//Dedicated archive of infinite size
			ExhaustiveArchive* temporaryArchive = new ExhaustiveArchive();
			double avgScore = evaluateRoot(*rootToProcess.second, generationNumber, mode, *privateLearningEnvironment, *temporaryArchive, this->params);

			{	// Store result Mutual exclusion zone
				std::lock_guard<std::mutex> lock(resultsMutex);
				results.insert({ avgScore, rootToProcess.second });
			}

			{	// Insertion archiveMap update mutual exclusion zone
				std::lock_guard<std::mutex> lock(archiveMapMutex);
				archiveMap.insert({ rootToProcess.first, temporaryArchive });
			}
		}

		// Archive merging (if it seems interesting
		this->mergeArchiveMap(archiveMergingMutex, archiveMapMutex, nextArchiveToMerge, archiveMap);
	}

	// Clean up
	delete privateLearningEnvironment;
}

void Learn::ParallelLearningAgent::mergeArchiveMap(std::mutex& archiveMergingMutex, std::mutex& archiveMapMutex, uint64_t& nextArchiveToMerge, std::map<uint64_t, ExhaustiveArchive*>& archiveMap) {
	// Try to lock
	std::unique_lock<std::mutex> lockMerging(archiveMergingMutex, std::try_to_lock);
	if (lockMerging.owns_lock()) {
		bool hasDoneSomething;
		do {
			hasDoneSomething = false;
			std::vector<ExhaustiveArchive*> archivesToMerge;
			{
				// Check if there are archives to merge (exclusion zone)
				std::lock_guard<std::mutex> lock_map(archiveMapMutex);
				while (!archiveMap.empty() && nextArchiveToMerge == archiveMap.begin()->first) {
					archivesToMerge.push_back(archiveMap.begin()->second);
					archiveMap.erase(nextArchiveToMerge);
					nextArchiveToMerge++;
				}
			}

			// If an archive (or more) can be merged
			if (!archivesToMerge.empty()) {
				// The merging is going to happen
				hasDoneSomething = true;

				// Merge archives one by one
				for (ExhaustiveArchive* archiveToMerge : archivesToMerge) {
					// Do the merge
					archiveToMerge->insertAllRecordings(this->archive);

					// Delete the merged archive
					delete archiveToMerge;
				}
			}
		} while (hasDoneSomething);
	}
}

void Learn::ParallelLearningAgent::evaluateAllRootsInParallel(uint64_t generationNumber, LearningMode mode, std::multimap<double, const TPG::TPGVertex*>& results) {
	// Create and fill the queue for distributing work among threads
	// each root is associated to its number in the list for enabling the 
	// determinism of stochastic archive storage.
	std::queue<std::pair<uint64_t, const TPG::TPGVertex*>> rootsToProcess;
	uint64_t idx = 0;
	for (const TPG::TPGVertex* root : this->tpg.getRootVertices()) {
		rootsToProcess.push({ idx++ , root });
	}

	// Create Archive Map
	std::map<uint64_t, ExhaustiveArchive*> archiveMap;

	// Create counter of root for which the archive was updated
	uint64_t nextArchiveToMerge = 0;

	// Create mutexes
	std::mutex rootsToProcessMutex;
	std::mutex resultsMutex;
	std::mutex archiveMapMutex;
	std::mutex archiveMergingMutex;

	// Create the threads
	std::vector<std::thread> threads;
	for (auto i = 0; i < (this->maxNbThreads - 1); i++) {
		threads.emplace_back(std::thread(&ParallelLearningAgent::slaveEvalRootThread, this,
			generationNumber, mode,
			std::ref(rootsToProcess), std::ref(rootsToProcessMutex),
			std::ref(results), std::ref(resultsMutex),
			std::ref(archiveMap), std::ref(archiveMapMutex),
			std::ref(nextArchiveToMerge), std::ref(archiveMergingMutex)
		));
	}

	// Work in the main thread also
	this->slaveEvalRootThread(generationNumber, mode,
		rootsToProcess, rootsToProcessMutex,
		results, resultsMutex,
		archiveMap, archiveMapMutex,
		nextArchiveToMerge, archiveMergingMutex);

	// Join the threads
	for (auto& thread : threads) {
		thread.join();
	}
}