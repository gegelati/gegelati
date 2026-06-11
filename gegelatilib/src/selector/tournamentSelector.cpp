/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2025) :
 *
 * Quentin Vacher <qvacher@insa-rennes.fr> (2025)
 *
 * GEGELATI is an open-source reinforcement learning framework for training
 * artificial intelligence based on Tangled Program Graphs (TPGs).
 *
 * This software is governed by the CeCILL-C license under French law and
 * abiding by the rules of distribution of free software. You can use,
 * modify and/ or redistribute the software under the terms of the CeCILL-C
 * license as circulated by CEA, CNRS and INRIA at the following URL
 * "http://www.cecill.info".
 *
 * As a counterpart to the access to the source code and rights to copy,
 * modify and redistribute granted by the license, users are provided only
 * with a limited warranty and the software's author, the holder of the
 * economic rights, and the successive licensors have only limited
 * liability.
 *
 * In this respect, the user's attention is drawn to the risks associated
 * with loading, using, modifying and/or developing or reproducing the
 * software by the user in light of its specific status of free software,
 * that may mean that it is complicated to manipulate, and that also
 * therefore means that it is reserved for developers and experienced
 * professionals having in-depth computer knowledge. Users are therefore
 * encouraged to load and test the software's suitability as regards their
 * requirements in conditions enabling the security of their systems and/or
 * data to be ensured and, more generally, to use and operate it in the
 * same conditions as regards security.
 *
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL-C license and that you accept its terms.
 */

#include "selector/tournamentSelector.h"

void Selector::TournamentSelector::launchSelection(
    std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                  const TPG::TPGVertex*>& results,
    Mutator::RNG& rng)
{

    // Clear the set of vertices to delete.
    this->verticesToDelete.clear();

    Selector::Selector::launchSelection(results, rng);
}

void Selector::TournamentSelector::doSelection(
    std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                  const TPG::TPGVertex*>& results,
    Mutator::RNG& rng)
{

    size_t nbToKeep =
        (size_t)(results.size() * params.selection.tournament.ratioSavedRoots);
    size_t nbAgentsInTournament = results.size() - nbToKeep;

    // Copy the first agents to remove (those at the bottom of the ranking)
    std::vector<std::pair<std::shared_ptr<Learn::EvaluationResult>,
                          const TPG::TPGVertex*>>
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
                      const TPG::TPGVertex*>
            subMap(subrangeBegin, subrangeEnd);

        // Delete everything but the best
        while (subMap.size() > 1) {
            auto itWorst = subMap.begin();
            erasedResults.push_back(itWorst->first);

            // Remove the vertex from the graph as well
            this->graph->removeVertex(*itWorst->second);

            subMap.erase(itWorst);
        }

        // This is a logical deletion, the vertex will be removed later
        this->addToVerticesToDelete(subMap.begin()->second);
    }

    // Delete from results and resultsPerRoot
    auto itDel = results.begin();
    for (size_t i = 0; i < nbAgentsInTournament && it != results.end(); ++i) {
        this->resultsPerRoot.erase(itDel->second);
        results.erase(itDel++);
    }
}

void Selector::TournamentSelector::addToVerticesToDelete(
    const TPG::TPGVertex* vertex)
{
    this->verticesToDelete.insert(vertex);
}

const Selector::SelectionContext& Selector::TournamentSelector::updateContext()
{
    Selector::updateContext();

    const auto& verticesToDeleteRef = this->verticesToDelete;

    // Erase the vertex set to be deleted to the list of pre existing vertex.
    // They are only used for being a new destination
    this->context.preExistingTeams.erase(
        std::remove_if(
            this->context.preExistingTeams.begin(),
            this->context.preExistingTeams.end(),
            [verticesToDeleteRef](const TPG::TPGVertex* vertex) -> bool {
                return verticesToDeleteRef.find(vertex) !=
                       verticesToDeleteRef.end();
            }),
        this->context.preExistingTeams.end());

    this->context.preExistingActions.erase(
        std::remove_if(
            this->context.preExistingActions.begin(),
            this->context.preExistingActions.end(),
            [verticesToDeleteRef](const TPG::TPGVertex* vertex) -> bool {
                return verticesToDeleteRef.find(vertex) !=
                       verticesToDeleteRef.end();
            }),
        this->context.preExistingActions.end());

    if (!params.selection.tournament.areElitesReproductible) {
        // The root not set to be deleted are not used during evolution
        this->context.teamsClonable.erase(
            std::remove_if(
                this->context.teamsClonable.begin(),
                this->context.teamsClonable.end(),
                [verticesToDeleteRef](const TPG::TPGVertex* vertex) -> bool {
                    return verticesToDeleteRef.find(vertex) ==
                           verticesToDeleteRef.end();
                }),
            this->context.teamsClonable.end());

        this->context.actionsClonable.erase(
            std::remove_if(
                this->context.actionsClonable.begin(),
                this->context.actionsClonable.end(),
                [verticesToDeleteRef](const TPG::TPGVertex* vertex) -> bool {
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
    this->context.nbActionsToCreate += this->context.actionsClonable.size();

    return context;
}

void Selector::TournamentSelector::updateAfterPopulate()
{
    // Remove vertex to be deleted
    for (auto vertex : this->verticesToDelete) {
        this->graph->removeVertex(*vertex);
    }
    this->verticesToDelete.clear();
}

const std::set<const TPG::TPGVertex*>& Selector::TournamentSelector::
    getVerticesToDelete()
{
    return this->verticesToDelete;
}
