

#include "selector/tournamentSelector.h"

void Selector::TournamentSelector::doSelection(
    std::shared_ptr<EvoGraph::Graph> graph,
    std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                  std::shared_ptr<const Algorithm::Agent>>& results,
    RNG::RNG& rng)
{
    this->agentsToDelete.clear();


    size_t nbToKeep =
        (size_t)(results.size() * params.selection.tournament.ratioSavedRoots);
    size_t nbAgentsInTournament = results.size() - nbToKeep;

    // Copy the first agents to remove (those at the bottom of the ranking)
    std::vector<std::pair<std::shared_ptr<Learn::EvaluationResult>,
                          std::shared_ptr<const Algorithm::Agent>>>
        elements;
    auto it = results.begin();
    for (size_t i = 0; i < nbAgentsInTournament && it != results.end();
         ++i, ++it) {
        elements.push_back(*it);
    }

    // Shuffle with custom RNG
    for (size_t i = elements.size() - 1; i > 0; --i) {
        size_t j = rng.getUnsignedInt64(0, i); // Random index in [0, i]
        std::swap(elements[i], elements[j]);
    }

    std::vector<std::shared_ptr<Learn::EvaluationResult>> erasedResults;

    // Tournament selection
    for (size_t i = 0; i < nbAgentsInTournament;
         i += params.selection.tournament.sizeTournament) {
        size_t end = std::min(
            static_cast<size_t>(i + params.selection.tournament.sizeTournament),
            nbAgentsInTournament);
        auto subrangeBegin = elements.begin() + i;
        auto subrangeEnd = elements.begin() + end;

        std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                      std::shared_ptr<const Algorithm::Agent>>
            subMap(subrangeBegin, subrangeEnd);

        // Delete everything but the best
        while (subMap.size() > 1) {
            auto itWorst = subMap.begin();
            erasedResults.push_back(itWorst->first);

            // Remove the vertex from the graph as well
            this->manager->deleteAgent(itWorst->second, graph);

            subMap.erase(itWorst);
        }

        // This is a logical deletion, the vertex will be removed later
        this->addToVerticesToDelete(subMap.begin()->second);
    }

    // Delete from results and resultsPerAgent
    auto itDel = results.begin();
    for (size_t i = 0; i < nbAgentsInTournament && it != results.end(); ++i) {
        this->resultsPerAgent.erase(itDel->second);
        results.erase(itDel++);
    }
}

void Selector::TournamentSelector::addToVerticesToDelete(
    std::shared_ptr<const Algorithm::Agent> agent)
{
    this->agentsToDelete.insert(agent);
}

const Selector::SelectionContext& Selector::TournamentSelector::updateContext()
{
    Selector::updateContext();

    const auto& verticesToDeleteRef = this->verticesToDelete;

    /*
    // Erase the vertex set to be deleted to the list of pre existing vertex.
    // They are only used for being a new destination
    this->context.preExistingTeams.erase(
        std::remove_if(
            this->context.preExistingTeams.begin(),
            this->context.preExistingTeams.end(),
            [verticesToDeleteRef](const EvoGraph::Vertex* vertex) -> bool {
                return verticesToDeleteRef.find(vertex) !=
                       verticesToDeleteRef.end();
            }),
        this->context.preExistingTeams.end());

    this->context.preExistingActions.erase(
        std::remove_if(
            this->context.preExistingActions.begin(),
            this->context.preExistingActions.end(),
            [verticesToDeleteRef](const EvoGraph::Vertex* vertex) -> bool {
                return verticesToDeleteRef.find(vertex) !=
                       verticesToDeleteRef.end();
            }),
        this->context.preExistingActions.end());

    if (!params.selection.tournament.areElitesReproductible) {
        // The agent not set to be deleted are not used during evolution
        this->context.teamsClonable.erase(
            std::remove_if(
                this->context.teamsClonable.begin(),
                this->context.teamsClonable.end(),
                [verticesToDeleteRef](const EvoGraph::Vertex* vertex) -> bool {
                    return verticesToDeleteRef.find(vertex) ==
                           verticesToDeleteRef.end();
                }),
            this->context.teamsClonable.end());

        this->context.actionsClonable.erase(
            std::remove_if(
                this->context.actionsClonable.begin(),
                this->context.actionsClonable.end(),
                [verticesToDeleteRef](const EvoGraph::Vertex* vertex) -> bool {
                    return verticesToDeleteRef.find(vertex) ==
                           verticesToDeleteRef.end();
                }),
            this->context.actionsClonable.end());
    }
    else {
        if (this->context.teamsClonable.size() > 0) {
            this->context.nbTeamsToCreate -=
                this->context.preExistingTeams.size();
        }
        if (this->context.actionsClonable.size() > 0) {
            this->context.nbActionsToCreate -=
                this->context.preExistingActions.size();
        }
    }

    this->context.nbTeamsToCreate += this->context.teamsClonable.size();
    this->context.nbActionsToCreate += this->context.actionsClonable.size();*/


    const auto& agentsToDeleteRef = this->agentsToDelete;

    this->context.preExistingAgents.erase(
        std::remove_if(
            this->context.preExistingAgents.begin(),
            this->context.preExistingAgents.end(),
            [agentsToDeleteRef](const std::shared_ptr<const Algorithm::Agent> agent) -> bool {
                return agentsToDeleteRef.find(agent) !=
                       agentsToDeleteRef.end();
            }),
        this->context.preExistingAgents.end());

    if (!params.selection.tournament.areElitesReproductible) {
        // The agent not set to be deleted are not used during evolution
        this->context.agentsClonable.erase(
            std::remove_if(
                this->context.agentsClonable.begin(),
                this->context.agentsClonable.end(),
                [agentsToDeleteRef](const std::shared_ptr<const Algorithm::Agent> agent) -> bool {
                    return agentsToDeleteRef.find(agent) ==
                           agentsToDeleteRef.end();
                }),
            this->context.agentsClonable.end());
    }
    else if (this->context.agentsClonable.size() > 0) {
        this->context.nbAgentsToCreate -=
            this->context.preExistingAgents.size();
    }

    this->context.nbAgentsToCreate += this->context.agentsClonable.size();


    return context;
}

void Selector::TournamentSelector::updateAfterPopulate(std::shared_ptr<EvoGraph::Graph> graph)
{
    // Remove vertex to be deleted
    for (auto agent : this->agentsToDelete) {
        this->manager->deleteAgent(agent, graph);
    }
    this->verticesToDelete.clear();
}

const std::set<std::shared_ptr<const Algorithm::Agent>>& Selector::TournamentSelector::
    getVerticesToDelete()
{
    return this->agentsToDelete;
}