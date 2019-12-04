#include <algorithm>
#include <iterator>
#include <thread>
#include <queue>
#include <mutex>

#include "mutator/rng.h"
#include "tpg/tpgExecutionEngine.h"

#include "learn/parallelLearningAgent.h"

double Learn::ParallelLearningAgent::evaluateRoot(TPG::TPGExecutionEngine& tee, const TPG::TPGVertex& root, uint64_t generationNumber, Learn::LearningMode mode, Learn::LearningEnvironment& le, const Learn::LearningParameters& params)
{
	// Init results
	double result = 0.0;

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

		// Create the TPGExecutionEngine
		TPG::TPGExecutionEngine tee(this->env, (mode == LearningMode::TRAINING) ? &this->archive : NULL);

		// Execute for all root
		for (const TPG::TPGVertex* root : this->tpg.getRootVertices()) {
			// Set the seed of the archive for this root.
			if (mode == LearningMode::TRAINING) {
				this->archive.setRandomSeed(this->rng.getUnsignedInt64(0, UINT64_MAX));
			}
			double avgScore = ParallelLearningAgent::evaluateRoot(tee, *root, generationNumber, mode, this->learningEnvironment, this->params);
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
	std::map<uint64_t, std::pair<double, const TPG::TPGVertex*>>& resultsPerRootMap, std::mutex& resultsPerRootMapMutex,
	std::map<uint64_t, size_t>& archiveSeeds,
	std::map<uint64_t, Archive*>& archiveMap, std::mutex& archiveMapMutex) {

	// Clone learningEnvironment
	LearningEnvironment* privateLearningEnvironment = this->learningEnvironment.clone();

	// Create a TPGExecutionEngine
	Environment privateEnv(this->env.getInstructionSet(), privateLearningEnvironment->getDataSources(), this->env.getNbRegisters());
	TPG::TPGExecutionEngine tee(privateEnv, NULL);

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
			//Dedicated archive for the root
			Archive* temporaryArchive = NULL;
			if (mode == LearningMode::TRAINING) {
				temporaryArchive = new Archive(params.archiveSize, params.archivingProbability, archiveSeeds.at(rootToProcess.first));
			}
			tee.setArchive(temporaryArchive);

			double avgScore = evaluateRoot(tee, *rootToProcess.second, generationNumber, mode, *privateLearningEnvironment, this->params);

			{	// Store result Mutual exclusion zone
				std::lock_guard<std::mutex> lock(resultsPerRootMapMutex);
				resultsPerRootMap.insert({ rootToProcess.first, { avgScore, rootToProcess.second } });
			}

			if (mode == LearningMode::TRAINING) {
				{	// Insertion archiveMap update mutual exclusion zone
					std::lock_guard<std::mutex> lock(archiveMapMutex);
					archiveMap.insert({ rootToProcess.first, temporaryArchive });
				}
			}
		}
	}

	// Clean up
	delete privateLearningEnvironment;
}

void Learn::ParallelLearningAgent::mergeArchiveMap(std::map<uint64_t, Archive*>& archiveMap)
{
	// Scan the archives backward, starting from the last to identify the 
	// last params.archiveSize recordings to keep (or less).
	auto reverseIterator = archiveMap.rbegin();

	uint64_t nbRecordings = 0;
	while (nbRecordings < this->params.archiveSize && reverseIterator != archiveMap.rend()) {
		nbRecordings += reverseIterator->second->getNbRecordings();
		reverseIterator++;
	}

	// Insert identified recordings into this->archive
	while (reverseIterator != archiveMap.rbegin()) {
		reverseIterator--;

		auto i = reverseIterator->first;

		// Skip recordings in the first archive if needed
		uint64_t recordingIdx = 0;
		while (nbRecordings > this->params.archiveSize) {
			recordingIdx++;
			nbRecordings--;
		}

		// Insert remaining recordings
		while (recordingIdx < reverseIterator->second->getNbRecordings()) {
			// Access in reverse order
			const ArchiveRecording& recording = reverseIterator->second->at(recordingIdx);
			// forced Insertion
			this->archive.addRecording(recording.prog, reverseIterator->second->getDataHandlers().at(recording.dataHash), recording.result, true);
			recordingIdx++;
		}
	}

	// delete all archives
	reverseIterator = archiveMap.rbegin();
	while (reverseIterator != archiveMap.rend()) {
		delete reverseIterator->second;
		reverseIterator++;
	}
}

void Learn::ParallelLearningAgent::evaluateAllRootsInParallel(uint64_t generationNumber, LearningMode mode, std::multimap<double, const TPG::TPGVertex*>& results) {
	// Create and fill the queue for distributing work among threads
	// each root is associated to its number in the list for enabling the 
	// determinism of stochastic archive storage.
	std::queue<std::pair<uint64_t, const TPG::TPGVertex*>> rootsToProcess;
	uint64_t idx = 0;

	// Fill also a map for seeding the Archive for each root
	std::map<uint64_t, size_t> archiveSeeds;

	for (const TPG::TPGVertex* root : this->tpg.getRootVertices()) {
		rootsToProcess.push({ idx , root });
		if (mode == LearningMode::TRAINING) {
			archiveSeeds.insert({ idx, this->rng.getUnsignedInt64(0, UINT64_MAX) });
		}
		idx++;
	}

	// Create Archive Map
	std::map<uint64_t, Archive*> archiveMap;
	// Create Map for results
	std::map<uint64_t, std::pair<double, const TPG::TPGVertex*>> resultsPerRootMap;

	// Create mutexes
	std::mutex rootsToProcessMutex;
	std::mutex resultsPerRootMutex;
	std::mutex archiveMapMutex;

	// Create the threads
	std::vector<std::thread> threads;
	for (auto i = 0; i < (this->maxNbThreads - 1); i++) {
		threads.emplace_back(std::thread(&ParallelLearningAgent::slaveEvalRootThread, this,
			generationNumber, mode,
			std::ref(rootsToProcess), std::ref(rootsToProcessMutex),
			std::ref(resultsPerRootMap), std::ref(resultsPerRootMutex),
			std::ref(archiveSeeds),
			std::ref(archiveMap), std::ref(archiveMapMutex)
		));
	}

	// Work in the main thread also
	this->slaveEvalRootThread(generationNumber, mode,
		rootsToProcess, rootsToProcessMutex,
		resultsPerRootMap, resultsPerRootMutex,
		archiveSeeds,
		archiveMap, archiveMapMutex);

	// Join the threads
	for (auto& thread : threads) {
		thread.join();
	}

	// Merge the results
	for (auto resultPerRoot : resultsPerRootMap) {
		results.insert(resultPerRoot.second);
	}

	// Merge the archives
	this->mergeArchiveMap(archiveMap);
}