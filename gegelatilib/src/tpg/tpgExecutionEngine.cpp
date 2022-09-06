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

const TPG::TPGEdge& TPG::TPGExecutionEngine::evaluateTeam(const TPGTeam& team)
{
    // Copy outgoing edge list
    const std::list<TPG::TPGEdge*>& outgoingEdges = team.getOutgoingEdges();

    // Note: No need to exclude previously visited edges as the graph is now
    // assumed to be acyclic.

#ifdef DEBUG
    std::cout << "New team :" << &team << std::endl;
#endif

    // Evaluate all TPGEdge
    // First
    TPGEdge* bestEdge = *outgoingEdges.begin();
    double bestBid = this->evaluateEdge(*bestEdge);
#ifdef DEBUG
    std::cout << "R = " << bestBid << "*" << std::endl;
#endif
    // Others
    for (auto iter = ++outgoingEdges.begin(); iter != outgoingEdges.end();
         iter++) {
        TPGEdge* edge = *iter;
        double bid = this->evaluateEdge(*edge);
#ifdef DEBUG
        std::cout << "R = " << bid;
#endif
        if (bid >= bestBid) {
#ifdef DEBUG
            std::cout << "*" << std::endl;
#endif
            bestEdge = edge;
            bestBid = bid;
        }
        else {
#ifdef DEBUG
            std::cout << std::endl;
#endif
        }
    }

    return *bestEdge;
}

const std::vector<const TPG::TPGVertex*> TPG::TPGExecutionEngine::
    executeFromRoot(const TPGVertex& root)
{
    const TPGVertex* currentVertex = &root;

    std::vector<const TPGVertex*> visitedVertices;
    visitedVertices.push_back(currentVertex);

    // Browse the TPG until a TPGAction is reached.
    while (dynamic_cast<const TPG::TPGTeam*>(currentVertex)) {
        // Get the next edge
        const TPGEdge& edge = this->evaluateTeam(*(TPGTeam*)currentVertex);
        // update currentVertex and backup in visitedVertex.
        currentVertex = edge.getDestination();
        visitedVertices.push_back(currentVertex);
    }

    return visitedVertices;
}
