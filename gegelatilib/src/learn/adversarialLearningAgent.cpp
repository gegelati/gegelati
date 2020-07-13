/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2020) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2020)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2019 - 2020)
 * Pierre-Yves Le Rolland-Raumer <plerolla@insa-rennes.fr> (2020)
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


#include <memory>
#include <gegelati.h>

std::multimap<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex*>
Learn::AdversarialLearningAgent::evaluateAllRoots(uint64_t generationNumber,
                                               Learn::LearningMode mode)
{
    // deactivates parallelism if le is not cloneable
    if(!this->learningEnvironment.isCopyable()){
        throw std::runtime_error("Not copyable environment. Exciting.");
    }
    std::multimap<std::shared_ptr<EvaluationResult>, const TPG::TPGVertex*>
            results;
    evaluateAllRootsInParallel(generationNumber, mode, results);
    return results;
}

void Learn::AdversarialLearningAgent::evaluateAllRootsInParallel(
        uint64_t generationNumber, LearningMode mode,
        std::multimap<std::shared_ptr<EvaluationResult>, const TPG::TPGVertex*>&
        results)
{
    // Create and fill the queue for distributing work among threads
    // each root is associated to its number in the list for enabling the
    // determinism of stochastic archive storage.
    auto jobsToProcess = makeJobs(mode);

    // Create Archive Map
    std::map<uint64_t, Archive*> archiveMap;

    // Create Map for results
    std::map<const TPG::TPGVertex*, std::shared_ptr<EvaluationResult>>
            resultsPerRootMap;

    // Create intermediate Map to gather threads resukts
    std::map<uint64_t, std::pair<std::shared_ptr<EvaluationResult>,
            std::shared_ptr<Job>>>
            jobPerResultsPerIdxMap;

    // Create mutexes
    std::mutex rootsToProcessMutex;
    std::mutex resultsPerRootMutex;
    std::mutex archiveMapMutex;

    // Create the threads
    std::vector<std::thread> threads;
    for (auto i = 0; i < (this->maxNbThreads - 1); i++) {
        threads.emplace_back(std::thread(
                &AdversarialLearningAgent::slaveEvalJobThread, this, generationNumber,
                mode, std::ref(jobsToProcess), std::ref(rootsToProcessMutex),
                std::ref(jobPerResultsPerIdxMap), std::ref(resultsPerRootMutex), std::ref(archiveMap),
                std::ref(archiveMapMutex)));
    }

    // Work in the main thread also
    this->slaveEvalJobThread(generationNumber, mode, jobsToProcess,
                             rootsToProcessMutex, jobPerResultsPerIdxMap,
                             resultsPerRootMutex, archiveMap,
                             archiveMapMutex);

    // Join the threads
    for (auto& thread : threads) {
        thread.join();
    }

    // Gather the results
    for(auto & resultPerJob : jobPerResultsPerIdxMap){
        // getting the AdversarialEvaluationResult that should be in this pair
        std::shared_ptr<AdversarialEvaluationResult> res =
                std::dynamic_pointer_cast<AdversarialEvaluationResult>
                        (resultPerJob.second.first);
        int rootIdx=0;
        for(auto root : resultPerJob.second.second->getRoots()){
            auto iterator = resultsPerRootMap.find(root);
            if(iterator==resultsPerRootMap.end()){
                // first time we encounter the results of this root
                resultsPerRootMap.emplace(root,
                        std::make_shared<EvaluationResult>
                                (EvaluationResult(res->getScoreOf(rootIdx),
                                        res->getNbEvaluation())));
            }else{
                // there is already a score for this root, let's do an addition
                (*iterator->second)+=EvaluationResult(res->getScoreOf(rootIdx),
                                                      res->getNbEvaluation());
            }
            rootIdx++;
        }
    }

    // Swaps the results for the final map
    for (auto& resultPerRoot : resultsPerRootMap) {
        results.emplace(resultPerRoot.second,resultPerRoot.first);
    }

    // Merge the archives
    this->mergeArchiveMap(archiveMap);
}


std::shared_ptr<Learn::EvaluationResult> Learn::AdversarialLearningAgent::evaluateJob(
        TPG::TPGExecutionEngine& tee, const Job& job,
        uint64_t generationNumber, Learn::LearningMode mode,
        LearningEnvironment& le) const
{
    auto& ale = (AdversarialLearningEnvironment&) le;


    // Init results
    auto results = std::make_shared<AdversarialEvaluationResult>(this->agentsPerEvaluation);

    // Evaluate nbIteration times
    for (auto i = 0; i < this->iterationsPerJob; i++) {
        // Compute a Hash
        Data::Hash<uint64_t> hasher;
        uint64_t hash = hasher(generationNumber) ^ hasher(i);

        // Reset the learning Environment
        ale.reset(hash, mode);

        uint64_t nbActions = 0;

        auto roots = job.getRoots();
        auto rootsIterator = roots.begin();

        while (!ale.isTerminal() &&
               nbActions < this->params.maxNbActionsPerEval) {
            // Get the action
            uint64_t actionID =
                    ((const TPG::TPGAction*)tee.executeFromRoot(**rootsIterator).back())
                            ->getActionID();
            // Do it
            ale.doAction(actionID);

            rootsIterator++;
            if(rootsIterator==roots.end()){
                // All the roots have played, let's go back to the first one
                rootsIterator=roots.begin();
                // Count actions : we have finished the turn
                nbActions++;
            }
        }

        // Update results
        *results += *std::dynamic_pointer_cast<EvaluationResult>(ale.getScores());
    }

    return results;
}

std::queue<std::shared_ptr<Learn::Job>>
Learn::AdversarialLearningAgent::makeJobs(Learn::LearningMode mode,
                                          TPG::TPGGraph *tpgGraph) {
    // sets the tpg to the Learning Agent's one if no one was specified
    tpgGraph = tpgGraph==nullptr?&tpg:tpgGraph;

    std::queue<std::shared_ptr<Learn::Job>> jobs;

    // registers nb of evaluations per root and sorts it
    std::multimap<size_t, const TPG::TPGVertex*> nbEvals;
    for(auto root : tpgGraph->getRootVertices()){
        nbEvals.emplace(0,root);
    }

    const TPG::TPGVertex * root;

    size_t index = 0;
    // while nbEvals still contains roots.
    // roots will be removed after been evaluated enough times (indeed any root
    // shall be evaluated nbIterationsPerPolicyEvaluation times or more)
    while(!nbEvals.empty()){
        uint64_t archiveSeed;
        archiveSeed = this->rng.getUnsignedInt64(0, UINT64_MAX);


        size_t nbEvalsThisRoot = nbEvals.begin()->first;
        root = nbEvals.begin()->second;

        auto job = std::make_shared<Learn::Job>(Learn::Job(index++,archiveSeed,{root}));

        // erases the old pair corresponding to this root
        nbEvals.erase(nbEvals.begin());
        size_t newNbEvalsThisRoot = nbEvalsThisRoot+iterationsPerJob;
        // only re-add the root in the map if it has not enough been evaluated
        if(newNbEvalsThisRoot<params.nbIterationsPerPolicyEvaluation) {
            nbEvals.emplace(nbEvalsThisRoot + iterationsPerJob, root);
        }

        // adding other roots in this job
        // we begin at 1 as there is already "root" in the job
        for(int i=1; i<agentsPerEvaluation; i++){
            if(nbEvals.size()==0){
                // there is no root that has not been evaluated enough times
                // left. We will take a root that is already in the job.
                // That's default behavior, it could be changed for the better.
                auto roots = job->getRoots();
                size_t position = rng.getUnsignedInt64(0,roots.size()-1);
                auto iterator = roots.begin();
                std::advance(iterator,position);
                root=*iterator;
                job->addRoot(root);
                continue;
            }

            // we're sure there are still roots to include in jobs
            size_t position = rng.getUnsignedInt64(0,nbEvals.size()-1);
            auto iterator = nbEvals.begin();
            std::advance(iterator,position);

            size_t nbEvalsThisRoot = iterator->first;
            root = iterator->second;
            job->addRoot(root);

            // erases the old pair corresponding to this root
            nbEvals.erase(iterator);
            size_t newNbEvalsThisRoot = nbEvalsThisRoot+iterationsPerJob;
            // only re-add the root in the map if it has not enough been evaluated
            if(newNbEvalsThisRoot<params.nbIterationsPerPolicyEvaluation) {
                nbEvals.emplace(nbEvalsThisRoot + iterationsPerJob, root);
            }
        }

        jobs.push(job);
    }

    return jobs;
}