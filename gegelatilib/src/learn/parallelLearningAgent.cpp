/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2025) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2022)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2020)
 * Pierre-Yves Le Rolland-Raumer <plerolla@insa-rennes.fr> (2020)
 * Quentin Vacher <qvacher@insa-rennes.fr> (2025)
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

#include <algorithm>
#include <iterator>
#include <mutex>
#include <queue>
#include <thread>

#include "mutator/rng.h"

#include "learn/evaluationResult.h"
#include "learn/parallelLearningAgent.h"

Learn::ParallelLearningAgent::~ParallelLearningAgent()   
{
    while(this->allCloneLearningEnvironments.size() > 1) {
        LearningEnvironment* env = this->allCloneLearningEnvironments.back();
        this->allCloneLearningEnvironments.pop_back();
        delete env;
    }
}

void Learn::ParallelLearningAgent::init(uint64_t seed, bool doGeneratePopulation)
{
    this->maxNbThreads = this->params->nbThreads;
    Learn::LearningAgent::init(seed, doGeneratePopulation);
}

std::multimap<std::shared_ptr<Learn::EvaluationResult>, std::reference_wrapper<const Representation::Agent>>
Learn::ParallelLearningAgent::evaluateCurrentRepresentationAgents(uint64_t generationNumber,
                                               Learn::LearningMode mode)
{


    if(this->currentExecutedRepresentation == nullptr){
        throw std::runtime_error("LearningAgent::evaluateOneRepresentationAgents: currentExecutedRepresentation is not set.");
    }
    if(!this->containsRepresentation(*this->currentExecutedRepresentation)){
        throw std::runtime_error("LearningAgent::evaluateOneRepresentationAgents: The learning agent does not contain the given representation.");
    }

    std::multimap<std::shared_ptr<EvaluationResult>, std::reference_wrapper<const Representation::Agent>>
        results;

        
    if (false && (this->maxNbThreads <= 1 || !this->learningEnvironment.isCopyable())) {
        results = Learn::LearningAgent::evaluateCurrentRepresentationAgents(generationNumber, mode);
    }
    else {
        // Create jobs to process
        std::vector<std::shared_ptr<Representation::Job>> jobsToProcess = makeJobs(mode);

        // Create a copy of jobsToProcess in a queue structure
        std::queue<std::shared_ptr<Representation::Job>> jobsQueue;
        for (const auto& job : jobsToProcess) {
            jobsQueue.push(job);
        }

        // Parallel mode
        evaluateAgentsInParallel(jobsQueue, generationNumber, mode, results);

        // Update the representation after evaluation with the jobs processed
        this->currentExecutedRepresentation->updateAfterEvaluation(jobsToProcess, mode);
    }

    return results;
}

void Learn::ParallelLearningAgent::slaveEvalJobThread(
    uint64_t generationNumber, Learn::LearningMode mode,
    std::queue<std::shared_ptr<Representation::Job>>& jobsToProcess,
    std::mutex& agentsToProcessMutex,
    std::map<uint64_t, std::pair<std::shared_ptr<EvaluationResult>,
                                 std::shared_ptr<Representation::Job>>>& resultsPerAgentMap,
    std::mutex& resultsPerAgentMapMutex,
    size_t indexEnvironment)
{

    // Clone learningEnvironment
    LearningEnvironment* privateLearningEnvironment = this->allCloneLearningEnvironments.at(indexEnvironment);

    std::unique_ptr<Representation::ExecutionEngine> execEngine = this->currentExecutedRepresentation->getManager().createExecutionEngine(privateLearningEnvironment->getDataSources());

    // Pop a job and process it
    while (true) {
        bool doProcess = false;
        std::shared_ptr<Representation::Job> jobToProcess;

        { // Mutual exclusion zone: atomic job acquisition + engine creation
            std::lock_guard<std::mutex> lock(agentsToProcessMutex);
            if (!jobsToProcess.empty()) {
                jobToProcess = jobsToProcess.front();
                jobsToProcess.pop();
                doProcess = true;
            }
        } // End of mutual exclusion zone

        // Exit if no more jobs
        if (!doProcess) {
            break;
        }

        // Evaluate the job with its dedicated engine
        std::shared_ptr<EvaluationResult> avgScore =
            this->evaluateJob(*execEngine, *jobToProcess, generationNumber, mode,
                              *privateLearningEnvironment);

        { // Store result Mutual exclusion zone
            std::lock_guard<std::mutex> lock(resultsPerAgentMapMutex);
            resultsPerAgentMap.emplace(
                jobToProcess->getIdx(),
                std::make_pair(avgScore, jobToProcess));
        }
    }
}

void Learn::ParallelLearningAgent::evaluateAgentsInParallel(
    std::queue<std::shared_ptr<Representation::Job>>& jobsToProcess, uint64_t generationNumber, LearningMode mode,
    std::multimap<std::shared_ptr<EvaluationResult>, std::reference_wrapper<const Representation::Agent>>&
        results)
{
    // Create Map for results
    std::map<uint64_t,
             std::pair<std::shared_ptr<EvaluationResult>, std::shared_ptr<Representation::Job>>>
        resultsPerJobMap;

    evaluateAgentsInParallelExecute(jobsToProcess, generationNumber, mode, resultsPerJobMap);

    evaluateAgentsInParallelCompileResults(resultsPerJobMap, results);

}
void Learn::ParallelLearningAgent::evaluateAgentsInParallelExecute(
    std::queue<std::shared_ptr<Representation::Job>>& jobsToProcess, uint64_t generationNumber, LearningMode mode,
    std::map<uint64_t, std::pair<std::shared_ptr<EvaluationResult>,
                                 std::shared_ptr<Representation::Job>>>& resultsPerJobMap)
{
    // Create mutexes
    std::mutex agentsToProcessMutex;
    std::mutex resultsPerAgentMutex;
    
    if(this->allCloneLearningEnvironments.size() == 0) {
        this->allCloneLearningEnvironments.push_back(&this->learningEnvironment);
        for(size_t idx = 0; idx < maxNbThreads - 1; idx++) {
            this->allCloneLearningEnvironments.push_back(this->learningEnvironment.clone());
        }
    }

    // Create the threads
    std::vector<std::thread> threads;
    for (auto i = 0; i < (this->maxNbThreads - 1); i++) {
        threads.emplace_back(std::thread(
            &ParallelLearningAgent::slaveEvalJobThread, this, generationNumber,
            mode, std::ref(jobsToProcess), std::ref(agentsToProcessMutex),
            std::ref(resultsPerJobMap), std::ref(resultsPerAgentMutex),
            i+1));
    }

    // Work in the main thread also, using the main environment
    this->slaveEvalJobThread(generationNumber, mode, jobsToProcess,
                             agentsToProcessMutex, resultsPerJobMap,
                             resultsPerAgentMutex, 0);

    // Join the threads
    for (auto& thread : threads) {
        thread.join();
    }
}

void Learn::ParallelLearningAgent::evaluateAgentsInParallelCompileResults(
    std::map<uint64_t, std::pair<std::shared_ptr<EvaluationResult>,
                                 std::shared_ptr<Representation::Job>>>& resultsPerJobMap,
    std::multimap<std::shared_ptr<EvaluationResult>, std::reference_wrapper<const Representation::Agent>>&
        results)
{
    // Merge the results
    for (auto& resultPerAgent : resultsPerJobMap) {
        results.emplace(resultPerAgent.second.first,
                        (*resultPerAgent.second.second).getAgent());
    }
}
