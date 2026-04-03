

#include "selector/tournamentSelector.h"

void Selector::TournamentSelector::doSelection(
    EvoGraph::Graph& graph,
    std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                  std::reference_wrapper<const Algorithm::Agent>>& results,
    RNG::RNG& rng)
{
    this->agentsToDelete.clear();


    size_t nbToKeep =
        (size_t)(results.size() * params->tournament.ratioSavedRoots);
    size_t nbAgentsInTournament = results.size() - nbToKeep;

    // Copy the first agents to remove (those at the bottom of the ranking)
    std::vector<std::pair<std::shared_ptr<Learn::EvaluationResult>,
                          std::reference_wrapper<const Algorithm::Agent>>>
        elements;
    
    for (size_t i = 0; i < nbAgentsInTournament && results.size() > 0; i++) {
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
    for (size_t i = 0; i < nbAgentsInTournament;
         i += params->tournament.sizeTournament) {
        size_t end = std::min(
            static_cast<size_t>(i + params->tournament.sizeTournament),
            nbAgentsInTournament);
        auto subrangeBegin = elements.begin() + i;
        auto subrangeEnd = elements.begin() + end;

        std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                      std::reference_wrapper<const Algorithm::Agent>>
            subMap(subrangeBegin, subrangeEnd);

        // Delete everything but the best
        while (subMap.size() > 1) {
            auto itWorst = subMap.begin();

            // Remove the vertex from the graph as well
            this->getManager().deleteAgent(itWorst->second, graph);
            

            subMap.erase(itWorst);
        }

        // This is a logical deletion, the vertex will be removed later
        this->addToVerticesToDelete(subMap.begin()->second);

    }
}

void Selector::TournamentSelector::addToVerticesToDelete(
    const Algorithm::Agent& agent)
{
    this->agentsToDelete.insert(agent);
}

std::unique_ptr<Selector::SelectionContext> Selector::TournamentSelector::updateContext() const 
{
    std::unique_ptr<SelectionContext> context = std::move(Selector::updateContext());

    const auto& agentsToDeleteRef = this->agentsToDelete;

    context->preExistingAgents.erase(
        std::remove_if(
            context->preExistingAgents.begin(),
            context->preExistingAgents.end(),
            [agentsToDeleteRef](const Algorithm::Agent& agent) -> bool {
                return agentsToDeleteRef.find(agent) !=
                       agentsToDeleteRef.end();
            }),
        context->preExistingAgents.end());

    if (!params->tournament.areElitesReproductible) {
        // The agent not set to be deleted are not used during evolution
        context->agentsClonable.erase(
            std::remove_if(
                context->agentsClonable.begin(),
                context->agentsClonable.end(),
                [agentsToDeleteRef](const Algorithm::Agent& agent) -> bool {
                    return agentsToDeleteRef.find(agent) ==
                           agentsToDeleteRef.end();
                }),
            context->agentsClonable.end());
    }
    else if (context->agentsClonable.size() > 0) {
        context->nbAgentsToCreate -=
            context->preExistingAgents.size();
    }

    context->nbAgentsToCreate += context->agentsClonable.size();

    return context;
}

void Selector::TournamentSelector::updateAfterPopulate(EvoGraph::Graph& graph)
{
    // Remove vertex to be deleted
    for (auto agent : this->agentsToDelete) {
        auto mapIterator = this->resultsPerAgent.find(agent);
        if (mapIterator != this->resultsPerAgent.end()) {
            this->removeFromSavedResults((*mapIterator).first);
        }

        this->getManager().deleteAgent(agent, graph);
    }
    this->agentsToDelete.clear();
}

const std::set<std::reference_wrapper<const Algorithm::Agent>>& Selector::TournamentSelector::
    getAgentsToDelete()
{
    return this->agentsToDelete;
}