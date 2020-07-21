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

#include "learn/adversarialLearningAgent.h"

std::multimap<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex*>
Learn::AdversarialLearningAgent::evaluateAllRoots(uint64_t generationNumber,
                                                  Learn::LearningMode mode)
{
    // deactivates parallelism if le is not cloneable
    if (!this->learningEnvironment.isCopyable()) {
        throw std::runtime_error("Not copyable environment. Exciting.");
    }
    std::multimap<std::shared_ptr<EvaluationResult>, const TPG::TPGVertex*>
        results;
    evaluateAllRootsInParallel(generationNumber, mode, results);
    return results;
}

void Learn::AdversarialLearningAgent::evaluateAllRootsInParallelCompileResults(
    std::map<uint64_t, std::pair<std::shared_ptr<EvaluationResult>,
                                 std::shared_ptr<Job>>>& resultsPerJobMap,
    std::multimap<std::shared_ptr<EvaluationResult>, const TPG::TPGVertex*>&
        results,
    std::map<uint64_t, Archive*>& archiveMap)
{
    // Create temporary map to gather results per root
    std::map<const TPG::TPGVertex*, std::shared_ptr<EvaluationResult>>
        resultsPerRootMap;

    // Gather the results
    for (const auto& resultPerJob : resultsPerJobMap) {
        // getting the AdversarialEvaluationResult that should be in this pair
        std::shared_ptr<AdversarialEvaluationResult> res =
            std::dynamic_pointer_cast<AdversarialEvaluationResult>(
                resultPerJob.second.first);
        int rootIdx = 0;
        for (auto root : std::dynamic_pointer_cast<Learn::AdversarialJob>(
                             resultPerJob.second.second)
                             ->getRoots()) {
            auto iterator = resultsPerRootMap.find(root);
            if (iterator == resultsPerRootMap.end()) {
                // first time we encounter the results of this root
                resultsPerRootMap.emplace(
                    root,
                    std::make_shared<EvaluationResult>(EvaluationResult(
                        res->getScoreOf(rootIdx), res->getNbEvaluation())));
            }
            else {
                // there is already a score for this root, let's do an addition
                (*iterator->second) += EvaluationResult(
                    res->getScoreOf(rootIdx), res->getNbEvaluation());
            }
            rootIdx++;
        }
    }

    // Swaps the results for the final map
    // It is important to iterate on tpg.getRootVertices : it ensures
    // the order of the roots iteration remains the same no matter
    // the order of resultsPerRootMap which depends on addresses.
    for (auto root : tpg.getRootVertices()) {
        auto& resultPerRoot = *resultsPerRootMap.find(root);
        results.emplace(resultPerRoot.second, resultPerRoot.first);
    }

    champions.clear();
    auto iterator = results.end();
    for(int i=0; i<=(1-params.ratioDeletedRoots)*tpg.getNbRootVertices()-1;i++){
        champions.emplace_back((--iterator)->second);
    }
    // Merge the archives
    this->mergeArchiveMap(archiveMap);
}

std::shared_ptr<Learn::EvaluationResult> Learn::AdversarialLearningAgent::
    evaluateJob(TPG::TPGExecutionEngine& tee, const Job& job,
                uint64_t generationNumber, Learn::LearningMode mode,
                LearningEnvironment& le) const
{
    auto& ale = (AdversarialLearningEnvironment&)le;

    // Init results
    auto results = std::make_shared<AdversarialEvaluationResult>(
        this->agentsPerEvaluation);

    // Evaluate nbIteration times
    for (auto i = 0; i < this->params.nbIterationsPerJob; i++) {
        // Compute a Hash
        Data::Hash<uint64_t> hasher;
        uint64_t hash = hasher(generationNumber) ^ hasher(i);

        // Reset the learning Environment
        ale.reset(hash, mode);

        uint64_t nbActions = 0;

        auto roots = ((AdversarialJob&)job).getRoots();

        auto rootsIterator = roots.begin();

        while (!ale.isTerminal() &&
               nbActions < this->params.maxNbActionsPerEval) {
            // Get the action
            uint64_t actionID =
                ((const TPG::TPGAction*)tee.executeFromRoot(*((TPG::TPGTeam*)*rootsIterator))
                     .back())
                    ->getActionID();
            // Do it
            ale.doAction(actionID);

            rootsIterator++;
            if (rootsIterator == roots.end()) {
                // All the roots have played, let's go back to the first one
                rootsIterator = roots.begin();
                // Count actions : we have finished the turn
                nbActions++;
            }
        }

        // Update results
        *results +=
            *std::dynamic_pointer_cast<EvaluationResult>(ale.getScores());
    }

    return results;
}

std::queue<std::shared_ptr<Learn::Job>> Learn::AdversarialLearningAgent::
    makeJobs(Learn::LearningMode mode, TPG::TPGGraph* tpgGraph)
{
    // sets the tpg to the Learning Agent's one if no one was specified
    tpgGraph = tpgGraph == nullptr ? &tpg : tpgGraph;

    std::queue<std::shared_ptr<Learn::Job>> jobs;

    size_t index=0;
    auto roots = tpg.getRootVertices();
    if(champions.size()==0){
        for(int i=0; i<roots.size()*(1-params.ratioDeletedRoots);i++){
            champions.emplace_back(roots[i]);
        }
    }

    for(auto root : roots){

        uint64_t archiveSeed;
        for(auto champion : champions) {
            archiveSeed = this->rng.getUnsignedInt64(0, UINT64_MAX);
            auto job = std::make_shared<Learn::AdversarialJob>(
                    Learn::AdversarialJob({root}, archiveSeed, index++));
            job->addRoot(champion);
            jobs.push(job);

            // do the same but with the champion as first agent
            archiveSeed = this->rng.getUnsignedInt64(0, UINT64_MAX);
            auto job2 = std::make_shared<Learn::AdversarialJob>(
                    Learn::AdversarialJob({champion}, archiveSeed, index++));
            job2->addRoot(root);
            jobs.push(job2);
        }
    }

    return jobs;
}