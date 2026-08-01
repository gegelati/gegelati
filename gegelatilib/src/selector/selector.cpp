

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

void Selector::Selector::setNbIndividuals(size_t nbIndividuals)
{
    this->nbIndividuals = nbIndividuals;
}

size_t Selector::Selector::getNbIndividuals()
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
    auto bestIndividualVertex = this->bestIndividual.first;
    if (bestIndividualVertex && population.containsIndividual(*bestIndividualVertex)) {

        // Remove all but the best individual from the graph
        while (population.getIndividuals().size() != 1) {
            auto individuals = population.getIndividuals();
            for (const Representation::Individual& individual : individuals) {
                if (individual != bestIndividualVertex.value()) {
                    population.deleteIndividual(individual, graph);
                }
            }
        }
    }
}

void Selector::Selector::removeFromSavedResults(const Representation::Individual& individual)
{
    if (&individual != nullptr) {
        this->resultsPerIndividual.erase(individual);
        if (this->bestIndividual.first && individual == *this->bestIndividual.first) {
            this->bestIndividual.first = std::nullopt;
            this->bestIndividual.second = nullptr;
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
    // Update bestIndividual
    this->updateBestIndividual(results);

    // Update resultsPerIndividual
    this->updateResultsPerIndividual(results);
}

void Selector::Selector::updateResultsPerIndividual(
    const std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                        std::reference_wrapper<const Representation::Individual>>& results)
{
    for (const auto& result : results) {
        auto mapIterator = this->resultsPerIndividual.find(result.second);
        if (mapIterator == this->resultsPerIndividual.end()) {
            // First time this individual is evaluated
            this->resultsPerIndividual.emplace(result.second, result.first);
        }
        else if (result.first != mapIterator->second) {
            // This individual has already been evaluated.
            // If the received result pointer is different from the one
            // stored in the map, update the one in the map by replacing it
            // with the new one (which was combined with the pre-existing
            // one in evalIndividual)
            mapIterator->second = result.first;
            // If the received result is associated to the current bestIndividual,
            // update it.
            if (result.second == *this->bestIndividual.first) {
                this->bestIndividual.second = result.first;
            }
        }
    }
}

void Selector::Selector::updateBestIndividual(
    const std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                        std::reference_wrapper<const Representation::Individual>>& results)
{
    auto iterator = --results.end();
    const std::shared_ptr<Learn::EvaluationResult> evaluation = iterator->first;
    const Representation::Individual& candidate = iterator->second;
    // Test the three replacement cases
    // from the simpler to the most complex to test
    if (!this->bestIndividual.first         // NULL case
        || *this->bestIndividual.second < *evaluation // new high-score case
        ||
        !this->getPopulation().containsIndividual(*this->bestIndividual.first) // bestIndividual disappearance
    ) {
        // Replace the best individual
        this->bestIndividual = {candidate, evaluation};
    }
}

const std::pair<std::optional<std::reference_wrapper<const Representation::Individual>>,
                std::shared_ptr<Learn::EvaluationResult>>&
Selector::Selector::getBestIndividual() const
{
    return this->bestIndividual;
}

void Selector::Selector::forgetPreviousResults()
{
    this->resultsPerIndividual.clear();
    this->bestIndividual.first = std::nullopt;
    this->bestIndividual.second = nullptr;
}

const std::map<std::reference_wrapper<const Representation::Individual>, std::shared_ptr<Learn::EvaluationResult>>&
Selector::Selector::getResultsPerIndividual() const
{
    return this->resultsPerIndividual;
}

std::unique_ptr<Selector::SelectionContext> Selector::Selector::updateContext() const
{
    std::unique_ptr<SelectionContext> context = std::make_unique<SelectionContext>();

    // Insert all individuals, but only the reference of weak pointer with lock available
    // population->getIndividuals returns a vector of weak pointer, but the context should only have reference to the individual, not the weak pointer itself, to avoid confusion in the mutation process where the weak pointer can be lock and unlock several times. Hence we insert the reference of the lock of the weak pointer in the context, but we do not insert the weak pointer itself.
    const Representation::Population& population = this->cGetPopulation();
    for (const Representation::Individual& individual : population.getIndividuals()) {
        context->individualsClonable.push_back(individual); 
        context->preExistingIndividuals.push_back(individual);
    }
    
    context->nbIndividualsToCreate =
        this->nbIndividuals - context->preExistingIndividuals.size();

    return context;
}


std::shared_ptr<Learn::EvaluationResult> Selector::Selector::getResultsOf(
    const Representation::Individual& individual) const
{
    // Has the root already been evaluated more times than
    // params.maxNbEvaluationPerPolicy
    const auto& iter = this->resultsPerIndividual.find(individual);
    if (iter != this->resultsPerIndividual.end()) {
        // The root has already been evaluated
        return iter->second;;
    } else {
        return nullptr;
    }
}

size_t Selector::Selector::getNbEvaluation(
    const Representation::Individual& individual) const
{
    // Has the root already been evaluated more times than
    // params.maxNbEvaluationPerPolicy
    const auto& iter = this->resultsPerIndividual.find(individual);
    if (iter != this->resultsPerIndividual.end()) {
        // The root has already been evaluated
        return iter->second->getNbEvaluation();
    }
    else {
        return 0;
    }
}