

#include "selector/selector.h"

void Selector::Selector::doSelection(
    std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                  std::shared_ptr<const Algorithm::Agent>>& results,
    RNG::RNG& rng)
{
    throw std::runtime_error(
        "Selection with main selector is not allowed, subclasses like "
        "TruncationSelector should be used.");
}

std::shared_ptr<Selector::SelectionMetrics> Selector::Selector::
    createSelectionMetrics() const
{
    return std::make_shared<SelectionMetrics>();
}

void Selector::Selector::keepBestPolicy()
{
    if (this->manager->containsAgent(this->bestAgent.first)) {
        auto bestAgentVertex = this->bestAgent.first;

        // Remove all but the best agent from the graph
        while (this->manager->getAgents().size() != 1) {
            auto agents = this->manager->getAgents();
            for (auto agent : agents) {
                if (agent != bestAgentVertex) {
                    this->manager->deleteAgent(agent, graph);
                }
            }
        }
    }
}

void Selector::Selector::updateEvaluationRecords(
    const std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                        std::shared_ptr<const Algorithm::Agent>>& results)
{
    // Update resultsPerAgent
    this->updateResultsPerAgent(results);

    // Update bestAgent
    this->updateBestAgent(results);
}

void Selector::Selector::updateResultsPerAgent(
    const std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                        std::shared_ptr<const Algorithm::Agent>>& results)
{
    for (auto result : results) {
        auto mapIterator = this->resultsPerAgent.find(result.second);
        if (mapIterator == this->resultsPerAgent.end()) {
            // First time this agent is evaluated
            this->resultsPerAgent.emplace(result.second, result.first);
        }
        else if (result.first != mapIterator->second) {
            // This agent has already been evaluated.
            // If the received result pointer is different from the one
            // stored in the map, update the one in the map by replacing it
            // with the new one (which was combined with the pre-existing
            // one in evalAgent)
            mapIterator->second = result.first;
            // If the received result is associated to the current bestAgent,
            // update it.
            if (result.second == this->bestAgent.first) {
                this->bestAgent.second = result.first;
            }
        }
    }
}

void Selector::Selector::updateBestAgent(
    const std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                        std::shared_ptr<const Algorithm::Agent>>& results)
{
    auto iterator = --results.end();
    const std::shared_ptr<Learn::EvaluationResult> evaluation = iterator->first;
    std::shared_ptr<const Algorithm::Agent> candidate = iterator->second;
    // Test the three replacement cases
    // from the simpler to the most complex to test
    if (this->bestAgent.first == nullptr         // NULL case
        || *this->bestAgent.second < *evaluation // new high-score case
        ||
        !this->manager->containsAgent(this->bestAgent.first) // bestAgent disappearance
    ) {
        // Replace the best agent
        this->bestAgent = {candidate, evaluation};
    }
}

const std::pair<std::shared_ptr<const Algorithm::Agent>,
                std::shared_ptr<Learn::EvaluationResult>>&
Selector::Selector::getBestAgent() const
{
    return this->bestAgent;
}

void Selector::Selector::forgetPreviousResults()
{
    this->resultsPerAgent.clear();
    this->bestAgent.first = nullptr;
    this->bestAgent.second = nullptr;
}

std::shared_ptr<EvoGraph::Graph> Selector::Selector::getGraph()
{
    return this->graph;
}

const std::map<std::shared_ptr<const Algorithm::Agent>, std::shared_ptr<Learn::EvaluationResult>>&
Selector::Selector::getResultsPerAgent() const
{
    return this->resultsPerAgent;
}

const Selector::SelectionContext& Selector::Selector::updateContext()
{
    // Get current vertex set (copy)
    auto vertices(graph->getVertices());
    // Get current agent teams (copy)
    auto agentVertices(graph->getRootVertices());

    // Create list of teams and actions clonable
    this->context.teamsClonable.clear();
    this->context.actionsClonable.clear();
    for (auto agent : agentVertices) {
        if (dynamic_cast<const EvoGraph::Team*>(agent) != nullptr) {
            this->context.teamsClonable.push_back((const EvoGraph::Team*)agent);
        }
        else if (params.mutation.tpg.useActionProgram) {
            this->context.actionsClonable.push_back(
                (const EvoGraph::Action*)agent);
        }
    }
    uint64_t nbAgentTeams = this->context.teamsClonable.size();
    uint64_t nbAgentActions = this->context.actionsClonable.size();

    // Fill the list of available Team and Actions, Actions are only
    // agents if they are not accessible by the teams
    this->context.preExistingTeams.clear();
    this->context.preExistingActions.clear();
    for (auto vertex : vertices) {
        if (dynamic_cast<const EvoGraph::Action*>(vertex) != nullptr &&
            (params.mutation.tpg.teamAccessAllActions ||
             vertex->getIncomingEdges().size() == 0)) {
            this->context.preExistingActions.push_back(
                (const EvoGraph::Action*)vertex);
        }
        else if (dynamic_cast<const EvoGraph::Team*>(vertex) != nullptr) {
            this->context.preExistingTeams.push_back(
                (const EvoGraph::Team*)vertex);
        }
    }

    // Fill the list of pre existing edges before mutations (copy)
    this->context.preExistingEdges.clear();
    auto& preExistingEdges = this->context.preExistingEdges;
    std::for_each(
        this->graph->getEdges().begin(), this->graph->getEdges().end(),
        [&preExistingEdges](const std::unique_ptr<EvoGraph::Edge>& edge) {
            preExistingEdges.push_back(edge.get());
        });

    this->context.nbTeamsToCreate =
        (uint64_t)(params.mutation.tpg.nbRoots *
                   params.mutation.tpg.ratioTeamsOverActions) -
        nbAgentTeams;

    this->context.nbActionsToCreate = std::max(
        (int64_t)((uint64_t)(params.mutation.tpg.nbRoots *
                             (1 - params.mutation.tpg.ratioTeamsOverActions)) -
                  nbAgentActions),
        (int64_t)0);

    return context;
}
