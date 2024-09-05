/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2022) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2022)
 * Thomas Bourgoin <tbourgoi@insa-rennes.fr> (2021)
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

#include <algorithm>
#include <set>
#include <vector>

#include "program/programExecutionEngine.h"
#include "tpg/tpgEdge.h"
#include "tpg/tpgTeam.h"

#include "tpg/tpgExecutionEngine.h"

void TPG::TPGExecutionEngine::setArchive(Archive* newArchive)
{
    this->archive = newArchive;
}

double TPG::TPGExecutionEngine::evaluateEdge(const TPGEdge& edge)
{
    // Get the program
    Program::Program& prog = edge.getProgram();

    // Set the progExecutionEngine to the program
    this->progExecutionEngine.setProgram(prog);

    // Execute the program.
    double result = this->progExecutionEngine.executeProgram();

    // Filter NaN results: replace with -inf
    result = (std::isnan(result)) ? -std::numeric_limits<double>::infinity()
                                  : result;

    // Put the result in the archive before returning it.
    if (this->archive != NULL) {
        this->archive->addRecording(&prog, progExecutionEngine.getDataSources(),
                                    result);
    }

    return result;
}

std::vector<const TPG::TPGEdge*> TPG::TPGExecutionEngine::executeTeam(
    const TPGVertex* currentTeam,
    std::vector<const TPGVertex*>& visitedVertices,
    std::vector<std::int64_t>* actionsTaken, uint64_t nbEdgesActivated)
{

    std::vector<const TPGEdge*> traversedEdges;

    // If "-1" is not anymore, it means all actions have been choosen.
    // Since we only consider the first action activated, we don't need to
    // execute more teams.
    if (actionsTaken && std::find(actionsTaken->begin(), actionsTaken->end(),
                                  -1) == actionsTaken->end()) {
        return traversedEdges;
    }

    // Add current team to the visited vertices.
    visitedVertices.push_back(currentTeam);

    // Copy outgoing edge list.
    const std::list<TPG::TPGEdge*>& outgoingEdges =
        currentTeam->getOutgoingEdges();

    uint64_t nbTeamActivated = 0;
    uint64_t nbEdgeActivated = 0;

    // Calcul the bids of all teams.
    std::vector<std::pair<TPG::TPGEdge*, double>> resultsBid;
    for (auto edge : outgoingEdges) {
        // Calcul program bid.
        double bid = this->evaluateEdge(*edge);
        resultsBid.push_back(std::make_pair(edge, bid));
    };

    // Sorting with ">=" is not possible, a sort with "<" then reverse is used
    // instead. Sort the results.
    std::sort(resultsBid.begin(), resultsBid.end(),
              [](const std::pair<TPG::TPGEdge*, double>& a,
                 const std::pair<TPG::TPGEdge*, double>& b) {
                  return a.second < b.second;
              });

    // Reverse
    std::reverse(resultsBid.begin(), resultsBid.end());

    size_t i = 0;
    // For all TPGEdge evaluated.
    while (i < resultsBid.size() && nbEdgeActivated < nbEdgesActivated) {
        nbEdgeActivated++;

        // Get the pair with the edge and the bid.
        auto destination = resultsBid[i].first->getDestination();

        // If edge destination is an action
        if (dynamic_cast<const TPGAction*>(destination)) {
            const TPGAction* action = (const TPGAction*)(destination);

            // Save the action value if the action ID is choosen for the first
            // time.
            if ((*actionsTaken)[action->getActionClass()] == -1) {
                (*actionsTaken)[action->getActionClass()] =
                    action->getActionID();
            }

            // Add the action the the visited vertices and the edge to the
            // traversed edges.
            visitedVertices.push_back(destination);
            traversedEdges.push_back(resultsBid[i].first);

            // Else if the no team has been activated yet.
        }
        else if (nbTeamActivated < 1) {
            nbTeamActivated++;

            // Only if the team has not already been visited.
            if (std::find(visitedVertices.begin(), visitedVertices.end(),
                          destination) == visitedVertices.end()) {

                // Add the edge to the traversed edges.
                traversedEdges.push_back(resultsBid[i].first);
                // If edge destination is a team, launch recursively the method.
                executeTeam((const TPGTeam*)(destination), visitedVertices,
                            actionsTaken, nbEdgesActivated);
            }
        }

        i++;
    }

    return traversedEdges;
}

std::pair<std::vector<const TPG::TPGVertex*>, std::vector<uint64_t>> TPG::
    TPGExecutionEngine::executeFromRoot(
        const TPGVertex& root, const std::vector<uint64_t>& initActions,
        uint64_t nbEdgesActivated)
{
    const TPGVertex* currentVertex = &root;
    std::vector<const TPGVertex*> visitedVertices;

    // An action value must be positive, so -1 for an action mean that no action
    // value is choosen yet.
    std::vector<std::int64_t> rawActionsTaken(initActions.size(), -1);

    // Execute the team only if it is really a team
    if(dynamic_cast<const TPGTeam*>(currentVertex)){
        executeTeam(dynamic_cast<const TPGTeam*>(currentVertex), visitedVertices,
                    &rawActionsTaken, nbEdgesActivated);
    } else {
        const TPGAction* action = (const TPGAction*)currentVertex;
        visitedVertices.push_back(currentVertex);
        rawActionsTaken[action->getActionClass()] = action->getActionID();
    }


    // Browse the raw list of actions and replace the "-1" action by the initial
    // value.
    std::vector<uint64_t> actionsTaken;
    for (uint64_t i = 0; i < rawActionsTaken.size(); i++) {
        if (rawActionsTaken[i] == -1) {
            actionsTaken.push_back((uint64_t)initActions[i]);
        }
        else {
            actionsTaken.push_back(rawActionsTaken[i]);
        }
    }

    auto results = std::make_pair(visitedVertices, actionsTaken);

    return results;
}
