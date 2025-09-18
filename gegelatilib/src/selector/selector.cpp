

#include "selector/selector.h"

void Selector::Selector::doSelection(std::multimap<std::shared_ptr<Learn::EvaluationResult>,
    const TPG::TPGVertex*>& results, Mutator::RNG& rng)
{
    throw std::runtime_error("Selection with main selector is not allowed, subclasses like TruncationSelector should be used.");    
}


void Selector::Selector::keepBestPolicy()
{
    // Evaluate all roots
    if (this->graph->hasVertex(*this->bestRoot.first)) {
        auto bestRootVertex = this->bestRoot.first;

        // Remove all but the best root from the graph
        while (this->graph->getNbRootVertices() != 1) {
            auto roots = this->graph->getRootVertices();
            for (auto root : roots) {
                if (root != bestRootVertex) {
                    graph->removeVertex(*root);
                }
            }
        }
    }
}

void Selector::Selector::updateEvaluationRecords(const std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                        const TPG::TPGVertex*>& results)
{
    // Update resultsPerRoot
    this->updateResultsPerRoot(results);

    // Update bestRoot
    this->updateBestRoot(results);

}

void Selector::Selector::updateResultsPerRoot(const std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                        const TPG::TPGVertex*>& results)
{
    for (auto result : results) {
        auto mapIterator = this->resultsPerRoot.find(result.second);
        if (mapIterator == this->resultsPerRoot.end()) {
            // First time this root is evaluated
            this->resultsPerRoot.emplace(result.second, result.first);
        }
        else if (result.first != mapIterator->second) {
            // This root has already been evaluated.
            // If the received result pointer is different from the one
            // stored in the map, update the one in the map by replacing it
            // with the new one (which was combined with the pre-existing
            // one in evalRoot)
            mapIterator->second = result.first;
            // If the received result is associated to the current bestRoot,
            // update it.
            if (result.second == this->bestRoot.first) {
                this->bestRoot.second = result.first;
            }
        }
    }
}

void Selector::Selector::updateBestRoot(const std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                        const TPG::TPGVertex*>& results)
{
    auto iterator = --results.end();
    const std::shared_ptr<Learn::EvaluationResult> evaluation = iterator->first;
    const TPG::TPGVertex* candidate = iterator->second;
    // Test the three replacement cases
    // from the simpler to the most complex to test
    if (this->bestRoot.first == nullptr         // NULL case
        || *this->bestRoot.second < *evaluation // new high-score case
        || !this->graph->hasVertex(
            *this->bestRoot.first) // bestRoot disappearance
    ) {
        // Replace the best root
        this->bestRoot = {candidate, evaluation};
    }
}

const std::pair<const TPG::TPGVertex*, std::shared_ptr<Learn::EvaluationResult>>& Selector::Selector::getBestRoot() const
{
    return this->bestRoot;
}

void Selector::Selector::forgetPreviousResults()
{
    this->resultsPerRoot.clear();
    this->bestRoot.first = nullptr;
    this->bestRoot.second = nullptr;
}

const std::map<const TPG::TPGVertex*, std::shared_ptr<Learn::EvaluationResult>>& Selector::Selector::getResultsPerRoot() const
{
    return this->resultsPerRoot;
}

const Selector::SelectionContext& Selector::Selector::updateContext()
{
    // Get current vertex set (copy)
    auto vertices(graph->getVertices());
    // Get current root teams (copy)
    auto rootVertices(graph->getRootVertices());

    // Create list of teams and actions clonable
    this->context.teamsClonable.clear();
    this->context.actionsClonable.clear();
    for(auto root: rootVertices){
        if (dynamic_cast<const TPG::TPGTeam*>(root) != nullptr) {
            this->context.teamsClonable.push_back((const TPG::TPGTeam*)root);
        }
        else if (params.mutation.tpg.useActionProgram) {
            this->context.actionsClonable.push_back((const TPG::TPGAction*)root);
        }
    
    }
    uint64_t nbRootTeams = this->context.teamsClonable.size();
    uint64_t nbRootActions = this->context.actionsClonable.size();

    // Fill the list of available TPGTeam and TPGActions, TPGActions are only roots if they are not accessible by the teams
    this->context.preExistingTeams.clear();
    this->context.preExistingActions.clear();
    for(auto vertex: vertices){
        if (dynamic_cast<const TPG::TPGAction*>(vertex) != nullptr && (params.mutation.tpg.teamAccessAllActions || vertex->getIncomingEdges().size() == 0)){
                this->context.preExistingActions.push_back((const TPG::TPGAction*)vertex);   
        }
        else if(dynamic_cast<const TPG::TPGTeam*>(vertex) != nullptr) {
            this->context.preExistingTeams.push_back((const TPG::TPGTeam*)vertex);
        }
    }

    // Fill the list of pre existing edges before mutations (copy)
    this->context.preExistingEdges.clear();
    auto& preExistingEdges = this->context.preExistingEdges;
    std::for_each(
        this->graph->getEdges().begin(), this->graph->getEdges().end(),
        [&preExistingEdges](const std::unique_ptr<TPG::TPGEdge>& edge) {
            preExistingEdges.push_back(edge.get());
        });


    bool useTournamentSelection = (params.selection.selectionMode == "tournament");

    if(useTournamentSelection){
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

    }


    this->context.nbTeamsToCreate =
        (uint64_t)(params.mutation.tpg.nbRoots * params.mutation.tpg.ratioTeamsOverActions) -
        nbRootTeams + (this->context.teamsClonable.size() * useTournamentSelection);

        
    this->context.nbActionsToCreate =
        std::max((int64_t)((uint64_t)(params.mutation.tpg.nbRoots *
                                        (1 - params.mutation.tpg.ratioTeamsOverActions)) -
                            nbRootActions +
                            (this->context.actionsClonable.size() * useTournamentSelection)),
                    (int64_t)0);

    return context;
}

