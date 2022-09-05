#include <deque>
#include <set>
#include <vector>

#include "learn/learningAgent.h"
#include "log/cycleDetectionLALogger.h"

void Log::CycleDetectionLALogger::logHeader()
{
    // nothing to log
}

void Log::CycleDetectionLALogger::logNewGeneration(uint64_t& generationNumber)
{
    // nothing to log
}

void Log::CycleDetectionLALogger::logAfterPopulateTPG()
{
    // Do a Depth First Search of the TPG from all roots.
    std::set<const TPG::TPGVertex*> visitedVertices;
    std::vector<const TPG::TPGVertex*> lifoToVisit;
    std::vector<const TPG::TPGVertex*> currentPath;

    // Add all roots to the set of vertex to visit
    const auto tpg = this->learningAgent.getTPGGraph();
    auto roots = tpg->getRootVertices();
    std::copy(roots.begin(), roots.end(), std::back_inserter(lifoToVisit));

    // Iterate on the stack
    while (!lifoToVisit.empty()) {
        // Pop first vertex
        const TPG::TPGVertex* vertex = lifoToVisit.back();
        lifoToVisit.pop_back();

        // If vertex is null pointer, this means we need to unstack a vertex
        // from the current path.
        if (vertex == nullptr) {
            currentPath.pop_back();
            // Go to the next iteration of the DFS while loop.
            continue;
        }

        // Mark vertex as visited
        visitedVertices.insert(vertex);

        // Scan outgoing edges
        const TPG::TPGTeam* team = dynamic_cast<const TPG::TPGTeam*>(vertex);
        if (team != nullptr) {
            // Push vertex in path
            currentPath.push_back(vertex);

            // Push a nullptr to lifo to visit. When this pointer is
            // encountered, the vertex will be popped from the current path.
            lifoToVisit.push_back(nullptr);

            for (auto edge : team->getOutgoingEdges()) {
                // Check if the destination is in the path
                if (std::find(currentPath.begin(), currentPath.end(),
                              edge->getDestination()) != currentPath.end()) {
                    // A cycle was detected !
                    *this << "A cycle was detected in the TPG.";

                    // Early termination
                    return;
                }

                // Put the destination on the lifo
                lifoToVisit.push_back(edge->getDestination());
            }
        }
    }

    // Check that all vertices were visited.
    if (visitedVertices.size() != tpg->getNbVertices()) {
        // Not all vertices were visited: A connected sub-graph was not visited.
        // This happens when a connected sub-graph has no root, which indicates
        // that the subgraph has a cycle.
        *this << "A cycle was detected in the TPG.";
    }

    // If this point of the algorithm is reached, no cycle were detected
    if (this->logOnSuccess) {
        *this << "No cycle detected in this TPG.";
    }
}

void Log::CycleDetectionLALogger::logAfterEvaluate(
    std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                  const TPG::TPGVertex*>& results)
{
    // nothing to log
}

void Log::CycleDetectionLALogger::logAfterDecimate()
{
    // nothing to log
}

void Log::CycleDetectionLALogger::logAfterValidate(
    std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                  const TPG::TPGVertex*>& results)
{
    // nothing to log
}

void Log::CycleDetectionLALogger::logEndOfTraining()
{
    // nothing to log
}
