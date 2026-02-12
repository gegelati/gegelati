

#include "selector/selector.h"

void Selector::Selector::doSelection(
    std::shared_ptr<EvoGraph::Graph> graph,
    std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                  std::weak_ptr<const Algorithm::Agent>>& results,
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

void Selector::Selector::keepBestPolicy(std::shared_ptr<EvoGraph::Graph> graph)
{
    auto bestAgentVertex = this->bestAgent.first;
    if (bestAgentVertex && this->manager->containsAgent(*bestAgentVertex)) {

        // Remove all but the best agent from the graph
        while (this->manager->getAgents().size() != 1) {
            auto agents = this->manager->getAgents();
            for (auto agent : agents) {
                if (agent.lock().get() != bestAgentVertex) {
                    this->manager->deleteAgent(*agent.lock(), graph);
                }
            }
        }
    }
}

void Selector::Selector::updateEvaluationRecords(
    const std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                        std::weak_ptr<const Algorithm::Agent>>& results)
{
    // Update resultsPerAgent
    this->updateResultsPerAgent(results);

    // Update bestAgent
    this->updateBestAgent(results);
}

void Selector::Selector::updateResultsPerAgent(
    const std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                        std::weak_ptr<const Algorithm::Agent>>& results)
{
    for (auto result : results) {
        auto mapIterator = this->resultsPerAgent.find(*result.second.lock());
        if (mapIterator == this->resultsPerAgent.end()) {
            // First time this agent is evaluated
            this->resultsPerAgent.emplace(*result.second.lock().get(), result.first);
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
            if (result.second.lock().get() == this->bestAgent.first) {
                this->bestAgent.second = result.first;
            }
        }
    }
}

void Selector::Selector::updateBestAgent(
    const std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                        std::weak_ptr<const Algorithm::Agent>>& results)
{
    auto iterator = --results.end();
    const std::shared_ptr<Learn::EvaluationResult> evaluation = iterator->first;
    std::weak_ptr<const Algorithm::Agent> candidate = iterator->second;
    // Test the three replacement cases
    // from the simpler to the most complex to test
    if (this->bestAgent.first == nullptr         // NULL case
        || *this->bestAgent.second < *evaluation // new high-score case
        ||
        !this->manager->containsAgent(*this->bestAgent.first) // bestAgent disappearance
    ) {
        // Replace the best agent
        this->bestAgent = {candidate.lock().get(), evaluation};
    }
}

const std::pair<const Algorithm::Agent*,
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

const std::map<std::reference_wrapper<const Algorithm::Agent>, std::shared_ptr<Learn::EvaluationResult>>&
Selector::Selector::getResultsPerAgent() const
{
    return this->resultsPerAgent;
}

std::unique_ptr<Selector::SelectionContext> Selector::Selector::updateContext() const
{
    std::unique_ptr<SelectionContext> context = std::make_unique<SelectionContext>();

    // Insert all agents, but only the reference of weak pointer with lock available
    // manager->getAgents returns a vector of weak pointer, but the context should only have reference to the agent, not the weak pointer itself, to avoid confusion in the mutation process where the weak pointer can be lock and unlock several times. Hence we insert the reference of the lock of the weak pointer in the context, but we do not insert the weak pointer itself.
    
    for (const auto& weakPtr : this->manager->getAgents()) {
        if (auto agentPtr = weakPtr.lock()) { 
            context->agentsClonable.push_back(*agentPtr); 
            context->preExistingAgents.push_back(*agentPtr);
        } 
    }
    
    context->nbAgentsToCreate =
        (uint64_t)(params.mutation.tpg.nbRoots) -
        context->preExistingAgents.size();

    return context;
}


bool Selector::Selector::isAgentEvalSkipped(
    std::weak_ptr<const Algorithm::Agent> agent,
    std::shared_ptr<Learn::EvaluationResult>& previousResult) const
{
    // Has the root already been evaluated more times than
    // params.maxNbEvaluationPerPolicy
    const auto& iter = this->resultsPerAgent.find(*agent.lock());
    if (iter != this->resultsPerAgent.end()) {
        // The root has already been evaluated
        previousResult = iter->second;
        return iter->second->getNbEvaluation() >=
               this->params.maxNbEvaluationPerPolicy;
    }
    else {
        previousResult = nullptr;
        return false;
    }
}