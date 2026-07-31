

#include "selector/selector.h"


Representation::AgentManager& Selector::Selector::getManager()
{
    if(!this->hasManager()) {
        throw std::runtime_error("Selector::getManager: manager is not set");
    }
    return *this->manager;
}

const Representation::AgentManager& Selector::Selector::cGetManager() const
{
    if(!this->hasManager()) {
        throw std::runtime_error("Selector::getManager: manager is not set");
    }
    return *this->manager;
}

void Selector::Selector::setManager(Representation::AgentManager& manager)
{
    this->manager = manager;
}

bool Selector::Selector::hasManager() const
{
    return this->manager.has_value();
}

void Selector::Selector::setNbAgents(size_t nbAgents)
{
    this->nbAgents = nbAgents;
}

size_t Selector::Selector::getNbAgents()
{
    return this->nbAgents;
}

void Selector::Selector::doSelection(
    EvoGraph::Graph& graph,
    std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                  std::reference_wrapper<const Representation::Agent>>& results,
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

void Selector::Selector::keepBestPolicy(EvoGraph::Graph& graph)
{
    Representation::AgentManager& manager = this->getManager();
    auto bestAgentVertex = this->bestAgent.first;
    if (bestAgentVertex && manager.containsAgent(*bestAgentVertex)) {

        // Remove all but the best agent from the graph
        while (manager.getAgents().size() != 1) {
            auto agents = manager.getAgents();
            for (const Representation::Agent& agent : agents) {
                if (agent != bestAgentVertex.value()) {
                    manager.deleteAgent(agent, graph);
                }
            }
        }
    }
}

void Selector::Selector::removeFromSavedResults(const Representation::Agent& agent)
{
    if (&agent != nullptr) {
        this->resultsPerAgent.erase(agent);
        if (this->bestAgent.first && agent == *this->bestAgent.first) {
            this->bestAgent.first = std::nullopt;
            this->bestAgent.second = nullptr;
        }
    }
    else {
        int a = 2;
    }
}

void Selector::Selector::updateEvaluationRecords(
    const std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                        std::reference_wrapper<const Representation::Agent>>& results)
{
    // Update bestAgent
    this->updateBestAgent(results);

    // Update resultsPerAgent
    this->updateResultsPerAgent(results);
}

void Selector::Selector::updateResultsPerAgent(
    const std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                        std::reference_wrapper<const Representation::Agent>>& results)
{
    for (const auto& result : results) {
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
            if (result.second == *this->bestAgent.first) {
                this->bestAgent.second = result.first;
            }
        }
    }
}

void Selector::Selector::updateBestAgent(
    const std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                        std::reference_wrapper<const Representation::Agent>>& results)
{
    auto iterator = --results.end();
    const std::shared_ptr<Learn::EvaluationResult> evaluation = iterator->first;
    const Representation::Agent& candidate = iterator->second;
    // Test the three replacement cases
    // from the simpler to the most complex to test
    if (!this->bestAgent.first         // NULL case
        || *this->bestAgent.second < *evaluation // new high-score case
        ||
        !this->getManager().containsAgent(*this->bestAgent.first) // bestAgent disappearance
    ) {
        // Replace the best agent
        this->bestAgent = {candidate, evaluation};
    }
}

const std::pair<std::optional<std::reference_wrapper<const Representation::Agent>>,
                std::shared_ptr<Learn::EvaluationResult>>&
Selector::Selector::getBestAgent() const
{
    return this->bestAgent;
}

void Selector::Selector::forgetPreviousResults()
{
    this->resultsPerAgent.clear();
    this->bestAgent.first = std::nullopt;
    this->bestAgent.second = nullptr;
}

const std::map<std::reference_wrapper<const Representation::Agent>, std::shared_ptr<Learn::EvaluationResult>>&
Selector::Selector::getResultsPerAgent() const
{
    return this->resultsPerAgent;
}

std::unique_ptr<Selector::SelectionContext> Selector::Selector::updateContext() const
{
    std::unique_ptr<SelectionContext> context = std::make_unique<SelectionContext>();

    // Insert all agents, but only the reference of weak pointer with lock available
    // manager->getAgents returns a vector of weak pointer, but the context should only have reference to the agent, not the weak pointer itself, to avoid confusion in the mutation process where the weak pointer can be lock and unlock several times. Hence we insert the reference of the lock of the weak pointer in the context, but we do not insert the weak pointer itself.
    const Representation::AgentManager& manager = this->cGetManager();
    for (const Representation::Agent& agent : manager.getAgents()) {
        context->agentsClonable.push_back(agent); 
        context->preExistingAgents.push_back(agent);
    }
    
    context->nbAgentsToCreate =
        this->nbAgents - context->preExistingAgents.size();

    return context;
}


std::shared_ptr<Learn::EvaluationResult> Selector::Selector::getResultsOf(
    const Representation::Agent& agent) const
{
    // Has the root already been evaluated more times than
    // params.maxNbEvaluationPerPolicy
    const auto& iter = this->resultsPerAgent.find(agent);
    if (iter != this->resultsPerAgent.end()) {
        // The root has already been evaluated
        return iter->second;;
    } else {
        return nullptr;
    }
}

size_t Selector::Selector::getNbEvaluation(
    const Representation::Agent& agent) const
{
    // Has the root already been evaluated more times than
    // params.maxNbEvaluationPerPolicy
    const auto& iter = this->resultsPerAgent.find(agent);
    if (iter != this->resultsPerAgent.end()) {
        // The root has already been evaluated
        return iter->second->getNbEvaluation();
    }
    else {
        return 0;
    }
}