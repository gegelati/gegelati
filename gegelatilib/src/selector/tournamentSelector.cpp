

#include "selector/tournamentSelector.h"

void Selector::TournamentSelector::doSelection(
    EvoGraph::Graph& graph,
    std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                  std::reference_wrapper<const Representation::Individual>>& results,
    RNG::RNG& rng)
{
    this->individualsToDelete.clear();


    size_t nbToKeep =
        (size_t)(results.size() * params->tournament.ratioSavedRoots);
    size_t nbIndividualsInTournament = results.size() - nbToKeep;

    // Copy the first individuals to remove (those at the bottom of the ranking)
    std::vector<std::pair<std::shared_ptr<Learn::EvaluationResult>,
                          std::reference_wrapper<const Representation::Individual>>>
        elements;
    
    for (size_t i = 0; i < nbIndividualsInTournament && results.size() > 0; i++) {
        elements.push_back(*results.begin());
        this->removeFromSavedResults(results.begin()->second);
        results.erase(results.begin());
    }

    // Shuffle with custom RNG
    for (size_t i = elements.size() - 1; i > 0; --i) {
        size_t j = rng.getUnsignedInt64(0, i); // Random index in [0, i]
        std::swap(elements[i], elements[j]);
    }


    // Tournament selection
    for (size_t i = 0; i < nbIndividualsInTournament;
         i += params->tournament.sizeTournament) {
        size_t end = std::min(
            static_cast<size_t>(i + params->tournament.sizeTournament),
            nbIndividualsInTournament);
        auto subrangeBegin = elements.begin() + i;
        auto subrangeEnd = elements.begin() + end;

        std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                      std::reference_wrapper<const Representation::Individual>>
            subMap(subrangeBegin, subrangeEnd);

        // Delete everything but the best
        while (subMap.size() > 1) {
            auto itWorst = subMap.begin();

            // Remove the vertex from the graph as well
            this->getPopulation().deleteIndividual(itWorst->second, graph);
            

            subMap.erase(itWorst);
        }

        // This is a logical deletion, the vertex will be removed later
        this->addToVerticesToDelete(subMap.begin()->second);

    }
}

void Selector::TournamentSelector::addToVerticesToDelete(
    const Representation::Individual& individual)
{
    this->individualsToDelete.insert(individual);
}

std::unique_ptr<Selector::SelectionContext> Selector::TournamentSelector::updateContext() const 
{
    std::unique_ptr<SelectionContext> context = std::move(Selector::updateContext());

    const auto& individualsToDeleteRef = this->individualsToDelete;

    context->preExistingIndividuals.erase(
        std::remove_if(
            context->preExistingIndividuals.begin(),
            context->preExistingIndividuals.end(),
            [individualsToDeleteRef](const Representation::Individual& individual) -> bool {
                return individualsToDeleteRef.find(individual) !=
                       individualsToDeleteRef.end();
            }),
        context->preExistingIndividuals.end());

    if (!params->tournament.areElitesReproductible) {
        // The individual not set to be deleted are not used during evolution
        context->individualsClonable.erase(
            std::remove_if(
                context->individualsClonable.begin(),
                context->individualsClonable.end(),
                [individualsToDeleteRef](const Representation::Individual& individual) -> bool {
                    return individualsToDeleteRef.find(individual) ==
                           individualsToDeleteRef.end();
                }),
            context->individualsClonable.end());
    }
    else if (context->individualsClonable.size() > 0) {
        context->nbIndividualsToCreate -=
            context->preExistingIndividuals.size();
    }

    context->nbIndividualsToCreate += context->individualsClonable.size();

    return context;
}

void Selector::TournamentSelector::updateAfterPopulate(EvoGraph::Graph& graph)
{
    // Remove vertex to be deleted
    for (auto individual : this->individualsToDelete) {
        auto mapIterator = this->resultsPerIndividual.find(individual);
        if (mapIterator != this->resultsPerIndividual.end()) {
            this->removeFromSavedResults((*mapIterator).first);
        }

        this->getPopulation().deleteIndividual(individual, graph);
    }
    this->individualsToDelete.clear();
}

const std::set<std::reference_wrapper<const Representation::Individual>>& Selector::TournamentSelector::
    getIndividualsToDelete()
{
    return this->individualsToDelete;
}