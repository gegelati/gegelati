

#include "selector/selector.h"


Representation::Population& Selector::Selector::getPopulation()
{
    if(!this->hasPopulation()) {
        throw std::runtime_error("Selector::getPopulation: population is not set");
    }
    return *this->population;
}

const Representation::Population& Selector::Selector::cGetPopulation() const
{
    if(!this->hasPopulation()) {
        throw std::runtime_error("Selector::getPopulation: population is not set");
    }
    return *this->population;
}

void Selector::Selector::setPopulation(Representation::Population& population)
{
    this->population = population;
}

bool Selector::Selector::hasPopulation() const
{
    return this->population.has_value();
}

void Selector::Selector::setNbAgents(size_t nbIndividuals)
{
    this->nbIndividuals = nbIndividuals;
}

size_t Selector::Selector::getNbAgents()
{
    return this->nbIndividuals;
}

void Selector::Selector::doSelection(
    EvoGraph::Graph& graph,
    std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                  std::reference_wrapper<const Representation::Individual>>& results,
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
    Representation::Population& population = this->getPopulation();
    auto bestAgentVertex = this->bestAgent.first;
    if (bestAgentVertex && population.containsAgent(*bestAgentVertex)) {

        // Remove all but the best agent from the graph
        while (population.getAgents().size() != 1) {
            auto agents = population.getAgents();
            for (const Representation::Individual& agent : agents) {
                if (agent != bestAgentVertex.value()) {
                    population.deleteAgent(agent, graph);
                }
            }
        }
    }
}

void Selector::Selector::removeFromSavedResults(const Representation::Individual& agent)
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
                        std::reference_wrapper<const Representation::Individual>>& results)
{
    // Update bestAgent
    this->updateBestAgent(results);

    // Update resultsPerAgent
    this->updateResultsPerAgent(results);
}

void Selector::Selector::updateResultsPerAgent(
    const std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                        std::reference_wrapper<const Representation::Individual>>& results)
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
                        std::reference_wrapper<const Representation::Individual>>& results)
{
    auto iterator = --results.end();
    const std::shared_ptr<Learn::EvaluationResult> evaluation = iterator->first;
    const Representation::Individual& candidate = iterator->second;
    // Test the three replacement cases
    // from the simpler to the most complex to test
    if (!this->bestAgent.first         // NULL case
        || *this->bestAgent.second < *evaluation // new high-score case
        ||
        !this->getPopulation().containsAgent(*this->bestAgent.first) // bestAgent disappearance
    ) {
        // Replace the best agent
        this->bestAgent = {candidate, evaluation};
    }
}

const std::pair<std::optional<std::reference_wrapper<const Representation::Individual>>,
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

const std::map<std::reference_wrapper<const Representation::Individual>, std::shared_ptr<Learn::EvaluationResult>>&
Selector::Selector::getResultsPerAgent() const
{
    return this->resultsPerAgent;
}

std::unique_ptr<Selector::SelectionContext> Selector::Selector::updateContext() const
{
    std::unique_ptr<SelectionContext> context = std::make_unique<SelectionContext>();

    // Insert all agents, but only the reference of weak pointer with lock available
    // population->getAgents returns a vector of weak pointer, but the context should only have reference to the agent, not the weak pointer itself, to avoid confusion in the mutation process where the weak pointer can be lock and unlock several times. Hence we insert the reference of the lock of the weak pointer in the context, but we do not insert the weak pointer itself.
    const Representation::Population& population = this->cGetPopulation();
    for (const Representation::Individual& agent : population.getAgents()) {
        context->agentsClonable.push_back(agent); 
        context->preExistingAgents.push_back(agent);
    }
    
    context->nbAgentsToCreate =
        this->nbIndividuals - context->preExistingAgents.size();

    return context;
}


std::shared_ptr<Learn::EvaluationResult> Selector::Selector::getResultsOf(
    const Representation::Individual& agent) const
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
    const Representation::Individual& agent) const
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