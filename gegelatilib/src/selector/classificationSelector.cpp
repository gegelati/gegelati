

#include "selector/classificationSelector.h"
#include "selector/classificationSelectionMetrics.h"

std::shared_ptr<Selector::SelectionMetrics> Selector::ClassificationSelector::
    createSelectionMetrics() const
{
    return std::make_shared<ClassificationSelectionMetrics>();
}

void Selector::ClassificationSelector::doSelection(
    EvoGraph::Graph& graph,
    std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                  std::reference_wrapper<const Representation::Individual>>& results,
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

    Representation::Population& population = this->getPopulation();

    // Compute the number of agent to keep/delete base on each criterion
    uint64_t totalNbAgent = population.getAgents().size();
    uint64_t nbAgentsToDelete = (uint64_t)floor(
        this->params->truncation.ratioDeletedRoots * totalNbAgent);
    uint64_t nbAgentsToKeep = (totalNbAgent - nbAgentsToDelete);

    // Keep ~half+ of the agents based on their general score on
    // all class.
    // and ~half- of the agents on a per class score (none if nbIndividuals to keep
    // < 2*nb class)
    uint64_t nbAgentsKeptPerClass = (nbAgentsToKeep / this->nbActions) / 2;
    uint64_t nbAgentsKeptGeneralScore =
        nbAgentsToKeep - this->nbActions * nbAgentsKeptPerClass;

    // Build a list of agents to keep
    std::vector<std::reference_wrapper<const Representation::Individual>> agentsToKeep;

    // Insert agents to keep per class
    for (uint64_t classIdx = 0; classIdx < this->nbActions; classIdx++) {
        // Fill a map with the agents and the score of the specific class as
        // ID.
        std::multimap<double, std::reference_wrapper<const Representation::Individual>> sortedAgent;
        std::for_each(
            results.begin(), results.end(),
            [&sortedAgent, &classIdx](
                const std::pair<std::shared_ptr<Learn::EvaluationResult>,
                                std::reference_wrapper<const Representation::Individual>>& res) {
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
            const Representation::Individual& agent = iterator->second;
            if (std::find_if(agentsToKeep.begin(), agentsToKeep.end(), 
                [&agent](const std::reference_wrapper<const Representation::Individual>& agentToKeep) {
                    return agent == agentToKeep.get();
                }) == agentsToKeep.end()) {
                agentsToKeep.push_back(iterator->second);
            }
            // Advance the iterator no matter what.
            iterator++;
        }

    // Insert remaining agents to keep
    auto iterator2 = results.rbegin();
    while (agentsToKeep.size() < nbAgentsToKeep && iterator2 != results.rend()) {
        // If the agent is not already marked to be kept
        const Representation::Individual& lockedAgent = iterator2->second;
        if (std::find_if(agentsToKeep.begin(), agentsToKeep.end(), 
            [&lockedAgent](const std::reference_wrapper<const Representation::Individual>& agent) {
                return agent.get() == lockedAgent;
            }) == agentsToKeep.end()) {
            agentsToKeep.push_back(iterator2->second);
        }
        // Advance the iterator no matter what.
        iterator2++;
    }

    // Do the removal.
    // Because of potential agent actions, the preserved number of agents
    // may be higher than the given ratio.
    auto allAgents = population.getAgents();
    auto& graphRef = graph;
    std::for_each(
        allAgents.begin(), allAgents.end(),
        [&agentsToKeep, &graphRef, this, &population,
         &results](std::reference_wrapper<const Representation::Individual> curragent) {

            if (std::find_if(agentsToKeep.begin(), agentsToKeep.end(), 
                [&curragent](const std::reference_wrapper<const Representation::Individual>& agent) {
                    return agent.get() == curragent.get();
                }) == agentsToKeep.end()) {
                population.deleteAgent(curragent, graphRef);

                // Keep only results of non-decimated agents.
                this->removeFromSavedResults(curragent);

                // Update results also
                std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                              std::reference_wrapper<const Representation::Individual>>::iterator iter =
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
}