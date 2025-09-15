

#include "selector/tournamentSelector.h"

void Selector::TournamentSelector::doSelection(std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                const TPG::TPGVertex*>& results, Mutator::RNG& rng)
{
    size_t nbToKeep =
        (size_t)(100 * params.tournament.ratioSavedRoots);
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
    for (size_t i = 0; i < nbAgentsInTournament; i += params.tournament.sizeTournament) {
        size_t end = std::min(static_cast<size_t>(i + params.tournament.sizeTournament),
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
        graph->setToBeDeleted(subMap.begin()->second);
    }

    // Delete the vertices marked for deletion
    for (const auto* v : toDelete) {
        graph->removeVertex(*v);
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