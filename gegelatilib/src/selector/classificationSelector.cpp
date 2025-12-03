

#include "selector/classificationSelector.h"
#include "selector/classificationSelectionMetrics.h"

std::shared_ptr<Selector::SelectionMetrics> Selector::ClassificationSelector::
    createSelectionMetrics() const
{
    return std::make_shared<ClassificationSelectionMetrics>();
}

void Selector::ClassificationSelector::doSelection(
    std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                  std::shared_ptr<const Algorithm::Agent>>& results,
    RNG::RNG& rng)
{
    // Check that results are ClassificationSelectionMetrics is used.
    // (also throws on empty results)
    const Learn::EvaluationResult* result = results.begin()->first.get();
    if (typeid(ClassificationSelectionMetrics) !=
        typeid(*result->getSelectionMetrics().get())) {
        throw std::runtime_error("Can not decimate worst agents for "
                                 "results whose metrics type is not "
                                 "ClassificationSelectionMetrics.");
    }

    // Compute the number of agent to keep/delete base on each criterion
    uint64_t totalNbAgent = this->manager->getAgents().size();
    uint64_t nbAgentsToDelete = (uint64_t)floor(
        this->params.selection.truncation.ratioDeletedRoots * totalNbAgent);
    uint64_t nbAgentsToKeep = (totalNbAgent - nbAgentsToDelete);

    // Keep ~half+ of the agents based on their general score on
    // all class.
    // and ~half- of the agents on a per class score (none if nbAgents to keep
    // < 2*nb class)
    uint64_t nbAgentsKeptPerClass = (nbAgentsToKeep / this->nbActions) / 2;
    uint64_t nbAgentsKeptGeneralScore =
        nbAgentsToKeep - this->nbActions * nbAgentsKeptPerClass;

    // Build a list of agents to keep
    std::vector<std::shared_ptr<const Algorithm::Agent>> agentsToKeep;

    // Insert agents to keep per class
    for (uint64_t classIdx = 0; classIdx < this->nbActions; classIdx++) {
        // Fill a map with the agents and the score of the specific class as
        // ID.
        std::multimap<double, std::shared_ptr<const Algorithm::Agent>> sortedAgent;
        std::for_each(
            results.begin(), results.end(),
            [&sortedAgent, &classIdx](
                const std::pair<std::shared_ptr<Learn::EvaluationResult>,
                                std::shared_ptr<const Algorithm::Agent>>& res) {
                sortedAgent.emplace(((ClassificationSelectionMetrics*)res.first
                                        ->getSelectionMetrics()
                                        .get())
                                       ->getScorePerClass()
                                       .at(classIdx),
                                   res.second);
            });

        // Keep the best nbAgentsKeptPerClass (or less for reasons explained
        // in the loop)
        auto iterator = sortedAgent.rbegin();
        for (auto i = 0; i < nbAgentsKeptPerClass; i++) {
            // If the agent is not already marked to be kept
            if (std::find(agentsToKeep.begin(), agentsToKeep.end(),
                          iterator->second) == agentsToKeep.end()) {
                agentsToKeep.push_back(iterator->second);
            }
            // Advance the iterator no matter what.
            // This means that if a agent scores well for several classes
            // it is kept only once anyway, but additional agents will not
            // be kept for any of the concerned class.
            iterator++;
        }
    }

    // Insert remaining agents to keep
    auto iterator = results.rbegin();
    while (agentsToKeep.size() < nbAgentsToKeep && iterator != results.rend()) {
        // If the agent is not already marked to be kept
        if (std::find(agentsToKeep.begin(), agentsToKeep.end(),
                      iterator->second) == agentsToKeep.end()) {
            agentsToKeep.push_back(iterator->second);
        }
        // Advance the iterator no matter what.
        iterator++;
    }

    // Do the removal.
    // Because of potential agent actions, the preserved number of agents
    // may be higher than the given ratio.
    auto allAgents = this->manager->getAgents();
    auto& graphRef = this->graph;
    auto& managerRef = this->manager;
    auto& resultsPerAgentRef = this->resultsPerAgent;
    std::for_each(
        allAgents.begin(), allAgents.end(),
        [&agentsToKeep, &graphRef, &resultsPerAgentRef, &managerRef,
         &results](std::shared_ptr<const Algorithm::Agent> curragent) {

            if (std::find(agentsToKeep.begin(), agentsToKeep.end(), curragent) ==
                    agentsToKeep.end()) {
                managerRef->deleteAgent(curragent, graphRef);

                // Keep only results of non-decimated agents.
                resultsPerAgentRef.erase(curragent);

                // Update results also
                std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                              std::shared_ptr<const Algorithm::Agent>>::iterator iter =
                    results.begin();
                while (iter != results.end()) {
                    if (iter->second == curragent) {
                        results.erase(iter);
                        break;
                    }
                    iter++;
                }
            }
        });
}