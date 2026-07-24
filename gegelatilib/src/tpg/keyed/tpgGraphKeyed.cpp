#include "tpg/keyed/tpgGraphKeyed.h"
#include "tpg/keyed/tpgEdgeKeyed.h"
#include "tpg/keyed/tpgKeyedFactory.h"
#include "tpg/keyed/tpgTeamKeyed.h"

void TPG::TPGGraphKeyed::setNewTeamKey(const TPGTeamKeyed& team,
                                       uint64_t newKey)
{
    // Find the team in the graph
    auto teamIterator = vertices.find(&team);

    if (teamIterator == vertices.end() || teamIterator->get() != &team) {
        throw std::runtime_error(
            "The team to modify does not exist in the TPGGraph.");
    }

    // Modify the key
    TPGTeamKeyed* rawPtr = dynamic_cast<TPGTeamKeyed*>(teamIterator->get());
    if (rawPtr == nullptr) {
        throw std::runtime_error("The team to modify is not a TPGTeamKeyed.");
    }
    rawPtr->setKey(newKey);
}

void TPG::TPGGraphKeyed::setNewEdgeLock(const TPG::TPGEdgeKeyed& edge,
                                        uint64_t newLock)
{
    // Find the edge in the graph
    auto edgeIterator = edges.find(&edge);
    if (edgeIterator == edges.end() || edgeIterator->get() != &edge) {
        throw std::runtime_error(
            "The edge to modify does not exist in the TPGGraph.");
    }
    // Modify the lock
    TPGEdgeKeyed* rawPtr = dynamic_cast<TPGEdgeKeyed*>(edgeIterator->get());
    if (rawPtr == nullptr) {
        throw std::runtime_error("The edge to modify is not a TPGEdgeKeyed.");
    }
    rawPtr->setLock(newLock);
}
