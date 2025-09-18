

#include "selector/tournamentSelector.h"

void Selector::TournamentSelector::doSelection(std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                const TPG::TPGVertex*>& results, Mutator::RNG& rng)
{
    size_t nbToKeep =
        (size_t)(params.mutation.tpg.nbRoots * params.selection.tournament.ratioSavedRoots);
    size_t nbAgentsInTournament = results.size() - nbToKeep;

    // Copy the first agents to remove (those at the bottom of the ranking)
    std::vector<
        std::pair<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex*>>
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

    std::vector<const TPG::TPGVertex*> toDelete;
    std::vector<std::shared_ptr<Learn::EvaluationResult>> erasedResults;

    // Tournament selection
    for (size_t i = 0; i < nbAgentsInTournament; i += params.selection.tournament.sizeTournament) {
        size_t end = std::min(static_cast<size_t>(i + params.selection.tournament.sizeTournament),
                              nbAgentsInTournament);
        auto subrangeBegin = elements.begin() + i;
        auto subrangeEnd = elements.begin() + end;

        std::multimap<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex*>
            subMap(subrangeBegin, subrangeEnd);

        // Delete everything but the best
        while (subMap.size() > 1) {
            auto itWorst = subMap.begin();
            toDelete.push_back(itWorst->second);
            erasedResults.push_back(itWorst->first);
            subMap.erase(itWorst);
        }

        // This is a logical deletion, the vertex will be removed later
        this->graph->setToBeDeleted(subMap.begin()->second);
    }

    // Delete the vertices marked for deletion
    for (const auto* v : toDelete) {
        this->graph->removeVertex(*v);
    }

    for (const auto* v : toDelete) {
        this->resultsPerRoot.erase(v);
    }

    // Delete from results and resultsPerRoot
    auto itDel = results.begin();
    for (size_t i = 0; i < nbAgentsInTournament && it != results.end(); ++i) {
        this->resultsPerRoot.erase(itDel->second);
        results.erase(itDel++);
    }
}

const Selector::SelectionContext& Selector::TournamentSelector::updateContext()
{
    Selector::updateContext();

    // The root not set to be deleted are not used during evolution
    this->context.teamsClonable.erase(
        std::remove_if(this->context.teamsClonable.begin(), this->context.teamsClonable.end(),
                    [](const TPG::TPGVertex* vertex) -> bool {
                        return !vertex->isToBeDeleted();
                    }),
        this->context.teamsClonable.end());

    this->context.actionsClonable.erase(
        std::remove_if(this->context.actionsClonable.begin(), this->context.actionsClonable.end(),
                    [](const TPG::TPGVertex* vertex) -> bool {
                        return !vertex->isToBeDeleted();
                    }),
        this->context.actionsClonable.end());

    // Erase the vertex set to be deleted to the list of pre existing vertex.
    // They are only used for being a new destination
    this->context.preExistingTeams.erase(
        std::remove_if(this->context.preExistingTeams.begin(), this->context.preExistingTeams.end(),
                    [](const TPG::TPGVertex* vertex) -> bool {
                        return vertex->isToBeDeleted();
                    }),
        this->context.preExistingTeams.end());

    this->context.preExistingActions.erase(
        std::remove_if(this->context.preExistingActions.begin(), this->context.preExistingActions.end(),
                    [](const TPG::TPGVertex* vertex) -> bool {
                        return vertex->isToBeDeleted();
                    }),
        this->context.preExistingActions.end());



    this->context.nbTeamsToCreate += this->context.teamsClonable.size();
    this->context.nbActionsToCreate += this->context.actionsClonable.size();

    return context;
}