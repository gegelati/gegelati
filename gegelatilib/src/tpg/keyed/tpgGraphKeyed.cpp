#include "tpg/keyed/tpgGraphKeyed.h"
#include "tpg/keyed/tpgEdgeKeyed.h"
#include "tpg/keyed/tpgKeyedFactory.h"
#include "tpg/keyed/tpgTeamKeyed.h"
#include <algorithm>
#include <stack>
#include <vector>

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

void TPG::TPGGraphKeyed::setNextTeamKey(const TPG::TPGTeamKeyed& team)
{
    // Find the team in the graph
    auto teamIterator = vertices.find(&team);
    if (teamIterator == vertices.end() || teamIterator->get() != &team) {
        throw std::runtime_error(
            "The team to modify does not exist in the TPGGraph.");
    }
    // Generate the next prime number
    auto isPrime = [](uint64_t num) {
        if (num <= 1)
            return false;
        if (num == 2)
            return true;
        for (uint64_t i = 3; i * i <= num; i += 2) {
            if (num % i == 0)
                return false;
        }
        return true;
    };

    uint64_t nextPrime = lastPrime;
    if (nextPrime == 1) {
        nextPrime = 2; // Start with the first prime number
    }
    else {
        if (nextPrime == 2) {
            nextPrime = 3; // Move to the next prime after 2
        }
        else {
            nextPrime += 2; // Start checking from the next odd number
        }
        while (!isPrime(nextPrime)) {
            nextPrime += 2;
        }
    }
    lastPrime = nextPrime;
    // Modify the key
    TPGTeamKeyed* rawPtr = dynamic_cast<TPGTeamKeyed*>(teamIterator->get());
    if (rawPtr == nullptr) {
        throw std::runtime_error("The team to modify is not a TPGTeamKeyed.");
    }
    rawPtr->setKey(nextPrime);
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

std::pair<std::set<const TPG::TPGTeamKeyed*>,
          std::set<const TPG::TPGEdgeKeyed*>>
TPG::TPGGraphKeyed::getSubtree(const TPGVertex& root,
                               std::set<uint64_t> keys) const
{
    // Create the ordered sets to store the teams and edges of the subtree
    std::set<const TPG::TPGTeamKeyed*> subtreeTeams;
    std::set<const TPG::TPGEdgeKeyed*> subtreeEdges;

    using Keys = std::set<uint64_t>;
    // Stack for DFS: pair of vertex pointer and keys available on the path to
    // it
    std::stack<std::pair<const TPG::TPGVertex*, Keys>> stack;
    stack.push({&root, Keys{keys}}); // start with the provided keys

    // Perform DFS traversal
    while (!stack.empty()) {
        auto currentPair = stack.top();
        stack.pop();

        const TPG::TPGVertex* current = currentPair.first;
        Keys keysAvailable = currentPair.second;

        // Check if the current vertex is a TPGTeamKeyed
        const TPG::TPGTeamKeyed* team =
            dynamic_cast<const TPG::TPGTeamKeyed*>(current);
        if (team) {
            subtreeTeams.insert(team);

            // Copy keys and add the key collected at this team so it's
            // available only for its subtree (children pushed below will use
            // this copy).
            Keys childKeys = keysAvailable;
            // Assume TPGTeamKeyed exposes getKey()
            childKeys.insert(team->getKey());

            // Iterate through the outgoing edges of the current vertex
            for (const TPG::TPGEdge* edge : team->getOutgoingEdges()) {
                // Check if the edge is a TPGEdgeKeyed and if it is unlocked by
                // any of the available keys for this subtree
                if (const TPG::TPGEdgeKeyed* keyedEdge =
                        dynamic_cast<const TPG::TPGEdgeKeyed*>(edge)) {
                    bool unlocked = std::any_of(
                        childKeys.begin(), childKeys.end(), [&](uint64_t k) {
                            return keyedEdge->isUnlockedByKey(k);
                        });

                    if (unlocked) {
                        subtreeEdges.insert(keyedEdge);
                        stack.push({keyedEdge->getDestination(), childKeys});
                    }
                }
            }
        }
        // else it is a TPGAction, we don't add it to the subtreeTeams set and
        // we don't explore outgoing edges from actions in this function.
    }

    return {subtreeTeams, subtreeEdges};
}
