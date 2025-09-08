/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2025) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019)
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

#include <algorithm>

#include "tpg/tpgActionEdge.h"
#include "tpg/tpgVertex.h"

const std::list<TPG::TPGEdge*>& TPG::TPGVertex::getIncomingEdges() const
{
    return this->incomingEdges;
}

const std::list<TPG::TPGEdge*>& TPG::TPGVertex::getOutgoingEdges() const
{
    return this->outgoingEdges;
}

void TPG::TPGVertex::addIncomingEdge(TPG::TPGEdge* edge)
{
    // Do nothing on NULL pointer
    if (edge != NULL) {
        // Add only if not already there
        if (std::find(this->incomingEdges.begin(), this->incomingEdges.end(),
                      edge) == this->incomingEdges.end()) {
            this->incomingEdges.push_back(edge);
        }
    }
}

void TPG::TPGVertex::removeIncomingEdge(TPG::TPGEdge* edge)
{
    // No need to do special checks on the given pointer.
    // at worse, nothing happens.
    this->incomingEdges.remove(edge);
}

void TPG::TPGVertex::addOutgoingEdge(TPG::TPGEdge* edge)
{
    // Do nothing on NULL pointer
    if (edge != NULL) {
        if (std::find(this->outgoingEdges.begin(), this->outgoingEdges.end(),
                      edge) == this->outgoingEdges.end()) {
            this->outgoingEdges.push_back(edge);
        }
    }
}

void TPG::TPGVertex::removeOutgoingEdge(TPG::TPGEdge* edge)
{
    this->outgoingEdges.remove(edge);
}

const std::set<uint64_t>& TPG::TPGVertex::getAssessedActions() const
{
    return this->assessedActions;
}

void TPG::TPGVertex::updateAssessedActions()
{
    assessedActions.clear();
    for (TPGEdge* edge : this->outgoingEdges) {
        if (auto* actionEdge = dynamic_cast<TPGActionEdge*>(edge)) {
            // If the edge is an action edge, insert its action class
            assessedActions.insert(actionEdge->getActionClass());
        }
        else if (edge != nullptr) {
            // Otherwise, insert all assessed actions from the destination
            const auto& destinationActions =
                edge->getDestination()->getAssessedActions();
            assessedActions.insert(destinationActions.begin(),
                                   destinationActions.end());
        }
    }
}

bool TPG::TPGVertex::hasSameAssessedActions(std::set<uint64_t> actions) const
{

    // Temporary set to store the intersection
    std::set<uint64_t> intersectionResult;

    // Compute the intersection
    std::set_intersection(
        actions.begin(), actions.end(), assessedActions.begin(),
        assessedActions.end(),
        std::inserter(intersectionResult, intersectionResult.begin()));

    return !intersectionResult.empty();
}

void TPG::TPGVertex::setToBeDeleted(bool status)
{
    this->toBeDeleted = status;
}

bool TPG::TPGVertex::isToBeDeleted() const
{
    return this->toBeDeleted;
}

uint64_t TPG::TPGVertex::getVertexID() const
{
    return this->vertexID;
}