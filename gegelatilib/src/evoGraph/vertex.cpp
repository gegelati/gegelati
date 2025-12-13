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

#include "evoGraph/actionEdge.h"
#include "evoGraph/vertex.h"

// Declaration of static vertex ID Counter in local here because it creates
// error in the .h file for MSVC compiler See:
// https://discourse.cmake.org/t/exporting-a-static-data-member-of-a-class-for-dll-using-msvc/5892
static uint64_t COUNT_VERTEX_ID = 0;

uint64_t EvoGraph::Vertex::incrementeCounter()
{
    return COUNT_VERTEX_ID++;
}

uint64_t EvoGraph::Vertex::getVertexIDCounter()
{
    return COUNT_VERTEX_ID;
}

void EvoGraph::Vertex::resetVertexIDCounter()
{
    COUNT_VERTEX_ID = 0;
}

const std::list<std::shared_ptr<const EvoGraph::Edge>>& EvoGraph::Vertex::getIncomingEdges() const
{
    return this->incomingEdges;
}

const std::list<std::shared_ptr<const EvoGraph::Edge>>& EvoGraph::Vertex::getOutgoingEdges() const
{
    return this->outgoingEdges;
}

void EvoGraph::Vertex::addIncomingEdge(std::shared_ptr<const Edge> edge)
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

void EvoGraph::Vertex::removeIncomingEdge(std::shared_ptr<const Edge> edge)
{
    // No need to do special checks on the given pointer.
    // at worse, nothing happens.
    this->incomingEdges.remove(edge);
}

void EvoGraph::Vertex::addOutgoingEdge(std::shared_ptr<const Edge> edge)
{
    // Do nothing on NULL pointer
    if (edge != NULL) {
        if (std::find(this->outgoingEdges.begin(), this->outgoingEdges.end(),
                      edge) == this->outgoingEdges.end()) {
            this->outgoingEdges.push_back(edge);
        }
    }
}

void EvoGraph::Vertex::removeOutgoingEdge(std::shared_ptr<const Edge> edge)
{
    this->outgoingEdges.remove(edge);
}

const std::set<uint64_t>& EvoGraph::Vertex::getAssessedActions() const
{
    return this->assessedActions;
}

void EvoGraph::Vertex::updateAssessedActions()
{
    assessedActions.clear();
    for (std::shared_ptr<const Edge> edge : this->outgoingEdges) {
        if (auto actionEdge = std::dynamic_pointer_cast<const ActionEdge>(edge)) {
            // If the edge is an action edge, insert its action class
            assessedActions.insert(actionEdge->getActionClass());
        }
        else {
            // Otherwise, insert all assessed actions from the destination
            const auto& destinationActions =
                edge->getDestination()->getAssessedActions();
            assessedActions.insert(destinationActions.begin(),
                                   destinationActions.end());
        }
    }
}

bool EvoGraph::Vertex::hasSameAssessedActions(std::set<uint64_t> actions) const
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

uint64_t EvoGraph::Vertex::getVertexID() const
{
    return this->vertexID;
}

void EvoGraph::Vertex::setVertexID(uint64_t newID)
{
    this->vertexID = newID;

    // Update the ID counter if needed
    if (newID >= COUNT_VERTEX_ID) {
        COUNT_VERTEX_ID = newID + 1;
    }
}

bool EvoGraph::operator<(const EvoGraph::Vertex& a, const EvoGraph::Vertex& b)
{
    return a.getVertexID() < b.getVertexID();
}