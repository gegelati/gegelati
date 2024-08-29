/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2020 - 2022) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2022)
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

#include <fstream>
#include <memory>

#include "learn/adversarialLearningAgent.h"

std::multimap<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex*>
Learn::AdversarialLearningAgent::evaluateAllRoots(uint64_t generationNumber,
                                                  Learn::LearningMode mode)
{
    // exception if LE is not cloneable and if there are several threads to use
    if (!this->learningEnvironment.isCopyable() && this->maxNbThreads > 1) {
        throw std::runtime_error(
            "Max number of threads for a non copyable environment is 1.");
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
        // Getting the AdversarialEvaluationResult that should be in this pair
        std::shared_ptr<AdversarialEvaluationResult> res =
            std::dynamic_pointer_cast<AdversarialEvaluationResult>(
                resultPerJob.second.first);

        auto advJob = std::dynamic_pointer_cast<Learn::AdversarialJob>(
            resultPerJob.second.second);
        // We browse the roots contained in the jobs to update their respective
        // scores, unless posOfStudiedRoot is defined. In this case, we will
        // only take 1 root in consideration.
        for (int i = std::max((int16_t)0, advJob->getPosOfStudiedRoot());
             i < advJob->getSize(); i++) {
            auto root = (*advJob)[i];
            auto iterator = resultsPerRootMap.find(root);
            if (iterator == resultsPerRootMap.end()) {
                // first time we encounter the results of this root
                resultsPerRootMap.emplace(
                    root, std::make_shared<EvaluationResult>(EvaluationResult(
                              res->getScoreOf(i), res->getNbEvaluation())));
            }
            else {
                // there is already a score for this root, let's do an addition
                (*iterator->second) += EvaluationResult(res->getScoreOf(i),
                                                        res->getNbEvaluation());
            }

            // if there is a specific root to read the score we skip the others
            // when it is done.
            if (advJob->getPosOfStudiedRoot() != -1) {
                break;
            }
        }
    }

    // Swaps the results for the final map
    // It is important to iterate on tpg.getRootVertices : it ensures
    // the order of the roots iteration remains the same no matter
    // the order of resultsPerRootMap which depends on addresses.
    for (auto root : tpg->getRootVertices()) {
        auto& resultPerRoot = *resultsPerRootMap.find(root);
        results.emplace(resultPerRoot.second, resultPerRoot.first);
    }

    champions.clear();
    auto iterator = results.end();
    for (int i = 0; i <= (1.0 - params.ratioDeletedRoots) *
                                 (double)tpg->getNbRootVertices() -
                             1.0;
         i++) {
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
            // Get the actions
            std::vector<uint64_t> actionsID 
                = tee.executeFromRoot(*((const TPG::TPGTeam*)*rootsIterator), le.getInitActions(), params.nbEdgesActivable).second; // TODO

            // Do it
            le.doActions(actionsID);

            rootsIterator++;
            if (rootsIterator == roots.end()) {
                // All the roots have played, let's go back to the first one
                rootsIterator = roots.begin();
                // Count actions : we have finished the turn
                nbActions++;
            }
        }

        auto scores = ale.getScores();

        // Update results
        *results += *std::dynamic_pointer_cast<EvaluationResult>(scores);
    }

    return results;
}

std::queue<std::shared_ptr<Learn::Job>> Learn::AdversarialLearningAgent::
    makeJobs(Learn::LearningMode mode, TPG::TPGGraph* tpgGraph)
{
    // sets the tpg to the Learning Agent's one if no one was specified
    tpgGraph = tpgGraph == nullptr ? tpg.get() : tpgGraph;

    std::queue<std::shared_ptr<Learn::Job>> jobs;

    size_t index = 0;

    auto roots = tpgGraph->getRootVertices();

    // if champions is empty fills it with the first roots come
    if (champions.size() == 0) {
        for (int i = 0;
             i <= (double)roots.size() * (1.0 - params.ratioDeletedRoots);
             i++) {
            champions.emplace_back(roots[i]);
        }
    }

    // Creates a list of teams of champion to compete with other roots.
    // We have to make enough teams to have nbIterationsPerPolicyEvaluation
    // iterations per root.
    int16_t nbChampionsTeams = (int16_t)std::ceil(
        (double)params.nbIterationsPerPolicyEvaluation /
        (double)(agentsPerEvaluation * params.nbIterationsPerJob));
    auto championsTeams =
        std::vector<std::vector<const TPG::TPGVertex*>>(nbChampionsTeams);

    // rng used to make champions teams
    Mutator::RNG rngChampions;
    for (auto& team : championsTeams) {
        // If the environment needs n agents, we will make lists of n-1
        // agents that will incorporate other roots.
        team = std::vector<const TPG::TPGVertex*>(agentsPerEvaluation - 1);
        for (int i = 0; i < agentsPerEvaluation - 1; i++) {
            auto it = champions.begin();
            std::advance(
                it, rngChampions.getUnsignedInt64(0, champions.size() - 1));
            team[i] = *it;
        }
    }

    // Each root is put at every possible location in champions teams
    // for example, let's say the champions team is A-B.
    // A root R will fulfill the list as follow :
    // -> 1 job with R-A-B
    // -> 1 job with A-R-B
    // -> 1 job with A-B-R
    for (auto root : roots) {
        uint64_t archiveSeed;
        // browses champions teams
        for (auto& team : championsTeams) {
            // puts the root at each possible location in the team
            for (int16_t i = 0; i < agentsPerEvaluation; i++) {
                archiveSeed = this->rng.getUnsignedInt64(0, UINT64_MAX);
                auto job = std::make_shared<Learn::AdversarialJob>(
                    Learn::AdversarialJob({}, archiveSeed, index++, i));

                for (int16_t j = 0; j < i; j++) {
                    job->addRoot(team[j]);
                }
                job->addRoot(root);
                for (int16_t j = i; j < agentsPerEvaluation - 1; j++) {
                    job->addRoot(team[j]);
                }

                jobs.push(job);
            }
        }
    }

    return jobs;
}

std::shared_ptr<Learn::Job> Learn::AdversarialLearningAgent::makeJob(
    const TPG::TPGVertex* vertex, Learn::LearningMode mode, int idx,
    TPG::TPGGraph* tpgGraph)
{
    throw std::runtime_error(
        "Method not supported in AdversarialLearningAgent.");
}
