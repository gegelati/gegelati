/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2022) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2022)
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
