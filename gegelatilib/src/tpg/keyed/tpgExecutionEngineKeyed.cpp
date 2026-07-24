#include "tpg/keyed/tpgExecutionEngineKeyed.h"
#include "tpg/keyed/tpgEdgeKeyed.h"
#include "tpg/keyed/tpgTeamKeyed.h"
#include <limits>
#include <stdexcept>

const TPG::TPGEdge& TPG::TPGExecutionEngineKeyed::evaluateTeam(
    const TPGTeam& team)
{
    // Try to cast to TPGTeamKeyed
    const TPGTeamKeyed* keyedTeam = dynamic_cast<const TPGTeamKeyed*>(&team);

    // Throw an error if the cast fails
    if (!keyedTeam) {
        throw std::runtime_error(
            "TPGExecutionEngineKeyed can only evaluate TPGTeamKeyed.");
    }

    // Add the team's key to collectedKeys
    uint64_t key = keyedTeam->getKey();
    if (key != 1) {
        collectedKeys.insert(key);
    }

    // Copy outgoing edge list
    std::list<TPG::TPGEdge*> outgoingEdges;
    std::copy(team.getOutgoingEdges().begin(), team.getOutgoingEdges().end(),
              std::back_inserter(outgoingEdges));

    // Note: No need to exclude previously visited edges as the graph is now
    // assumed to be acyclic.

    // exclude all TPGEdgeKeyed whose lock is not a multiple of the key of the
    // collectedKeys set.
    std::list<TPG::TPGEdge*> filteredEdges;
    outgoingEdges.remove_if([this](TPG::TPGEdge* edge) {
        const TPGEdgeKeyed* keyedEdge = dynamic_cast<const TPGEdgeKeyed*>(edge);
        if (!keyedEdge) {
            // Throw an error if the cast fails
            throw std::runtime_error(
                "TPGExecutionEngineKeyed can only evaluate "
                "TPGEdgeKeyed.");
        }

        uint64_t edgeLock = keyedEdge->getLock();
        for (const auto& key : collectedKeys) {
            if (edgeLock == 1 || edgeLock % key == 0) {
                return false; // Keep this edge
            }
        }
        return true; // Exclude this edge
    });

    // Throw an error if no edges are left after filtering
    if (outgoingEdges.empty()) {
        throw std::runtime_error(
            "No outgoing edges available for evaluation after filtering "
            "by collected keys.");
    }

    // Evaluate all TPGEdge
    // First
    TPGEdge* bestEdge = *outgoingEdges.begin();
    double bestBid = this->evaluateEdge(*bestEdge);

    // Others
    for (auto iter = ++outgoingEdges.begin(); iter != outgoingEdges.end();
         iter++) {
        TPGEdge* edge = *iter;
        double bid = this->evaluateEdge(*edge);

        if (bid >= bestBid) {
            bestEdge = edge;
            bestBid = bid;
        }
    }

    return *bestEdge;
}

const std::pair<std::vector<const TPG::TPGVertex*>, std::vector<double>> TPG::
    TPGExecutionEngineKeyed::executeFromRoot(
        const TPG::TPGVertex& root, const std::vector<uint64_t>& initActions)
{
    // Clear collected keys before starting execution
    clearCollectedKeys();

    // Call base implementation
    return TPGExecutionEngine::executeFromRoot(root, initActions);
}

const std::set<uint64_t> TPG::TPGExecutionEngineKeyed::getCollectedKeys() const
{
    return collectedKeys;
}

void TPG::TPGExecutionEngineKeyed::clearCollectedKeys()
{
    collectedKeys.clear();
}
