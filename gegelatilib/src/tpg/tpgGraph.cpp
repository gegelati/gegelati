/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2025) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2023)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2019 - 2020)
 * Quentin Vacher <qvacher@insa-rennes.fr> (2024 - 2025)
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

#include <queue>

#include <algorithm>
#include <stdexcept>
#include <type_traits>

#include "tpg/tpgGraph.h"

TPG::TPGGraph::~TPGGraph()
{
    // Just delete the vertices.
    // Edges will be deleted during their container destruction.
    for (TPG::TPGVertex* vertex : this->vertices) {
        delete vertex;
    }
}

TPG::TPGGraph& TPG::TPGGraph::operator=(TPGGraph model)
{
    swap(*this, model);
    return *this;
}

void TPG::TPGGraph::clear()
{
    // Remove all vertices
    while (this->vertices.size() > 0) {
        this->removeVertex(*this->vertices.front());
    }
}

const Environment& TPG::TPGGraph::getEnvironment() const
{
    return this->env;
}

const TPG::TPGFactory& TPG::TPGGraph::getFactory() const
{
    return *this->factory;
}

const TPG::TPGTeam& TPG::TPGGraph::addNewTeam()
{
    this->vertices.push_back(factory->createTPGTeam(countVertexID));
    this->mapVertexIDs.insert({countVertexID++, this->vertices.back()});
    return (const TPGTeam&)(*this->vertices.back());
}

const TPG::TPGAction& TPG::TPGGraph::addNewAction(uint64_t actionID)
{
    this->vertices.push_back(factory->createTPGAction(countVertexID, actionID));
    this->mapVertexIDs.insert({countVertexID++, this->vertices.back()});
    return (const TPGAction&)(*this->vertices.back());
}

size_t TPG::TPGGraph::getNbVertices() const
{
    return this->vertices.size();
}

const std::vector<const TPG::TPGVertex*> TPG::TPGGraph::getVertices() const
{
    std::vector<const TPG::TPGVertex*> result(this->vertices.size());
    std::copy(this->vertices.begin(), this->vertices.end(), result.begin());
    return result;
}

uint64_t TPG::TPGGraph::getNbRootVertices() const
{
    return std::count_if(this->vertices.begin(), this->vertices.end(),
                         [](const TPGVertex* vertex) {
                             return vertex->getIncomingEdges().size() == 0;
                         });
}

const std::vector<const TPG::TPGVertex*> TPG::TPGGraph::getRootVertices() const
{
    std::vector<const TPG::TPGVertex*> result;
    std::copy_if(this->vertices.begin(), this->vertices.end(),
                 std::back_inserter(result), [](TPGVertex* vertex) {
                     return vertex->getIncomingEdges().size() == 0;
                 });
    return result;
}

bool TPG::TPGGraph::hasVertex(const TPG::TPGVertex& vertex) const
{
    return std::find(this->vertices.cbegin(), this->vertices.cend(), &vertex) !=
           this->vertices.cend();
}

void TPG::TPGGraph::removeVertex(const TPGVertex& vertex)
{
    // Remove the vertex based on a pointer comparison.
    auto vertexPtr = this->findVertex(&vertex);
    if (vertexPtr != nullptr) {
        // Remove all connected edges.
        // copy inEdges set for removal
        // (because iterating on the modified set is not a good idea).
        std::list<TPGEdge*> inEdgesToRemove = vertexPtr->getIncomingEdges();
        for (auto inEdge : inEdgesToRemove) {
            this->removeEdge(*inEdge);
        }
        // copy outEdges set for removal
        std::list<TPGEdge*> outEdgesToRemove = vertexPtr->getOutgoingEdges();
        for (auto outEdge : outEdgesToRemove) {
            this->removeEdge(*outEdge);
        }
    }

    // Remove edge for action can launch again remove vertex.
    // Check again if the vertex is in the graph before deleting.
    vertexPtr = this->findVertex(&vertex);
    if (vertexPtr != nullptr) {
        // Free the memory of the vertex
        delete vertexPtr;
        // Remove the pointer from the list.
        this->vertices.remove(vertexPtr);
        // Remove the ID from the set of used IDs
        this->mapVertexIDs.erase(vertex.getVertexID());
    }
}

const TPG::TPGVertex& TPG::TPGGraph::cloneVertex(const TPGVertex& vertex)
{
    // Check that the vertex to clone exists in the graph
    auto vertexPtr = this->findVertex(&vertex);
    if (vertexPtr == nullptr) {
        throw std::runtime_error(
            "The vertex to clone does not exist in the TPGGraph.");
    }

    // Create a new Vertex
    // (at the end of the vertices list)
    if (dynamic_cast<const TPG::TPGTeam*>(&vertex) != nullptr) {
        this->addNewTeam();
    }
    else if (dynamic_cast<const TPG::TPGAction*>(&vertex) != nullptr) {
        this->addNewAction(((TPGAction&)vertex).getActionID());
    }

    // Get the new vertex
    TPGVertex* newVertex = this->vertices.back();

    // Copy the outgoing edges (if any).
    for (auto edge : vertex.getOutgoingEdges()) {

        if (dynamic_cast<TPG::TPGActionEdge*>(edge) != nullptr) {

            // If action edge, create new action edge, else create new standard
            // edge.
            TPG::TPGActionEdge* actionEdge = dynamic_cast<TPGActionEdge*>(edge);
            this->addNewActionEdge(*newVertex,
                                   actionEdge->getProgramSharedPointer(),
                                   actionEdge->getActionClass());
        }
        else if (edge != nullptr) {
            this->addNewEdge(*newVertex, *(edge->getDestination()),
                             edge->getProgramSharedPointer());
        }
        else {
            throw std::runtime_error("Edge copied should not be a nullptr.");
        }
    }

    newVertex->updateAssessedActions();

    return *newVertex;
}

const TPG::TPGEdge& TPG::TPGGraph::addNewEdge(
    const TPGVertex& src, const TPGVertex& dest,
    const std::shared_ptr<Program::Program> prog)
{
    // Check the TPGVertex existence within the graph.
    auto srcVertex =
        std::find_if(this->vertices.begin(), this->vertices.end(),
                     [&src](TPG::TPGVertex* other) { return other == &src; });
    auto dstVertex =
        std::find_if(this->vertices.begin(), this->vertices.end(),
                     [&dest](TPG::TPGVertex* other) { return other == &dest; });
    if (dstVertex == this->vertices.end() ||
        srcVertex == this->vertices.end()) {
        throw std::runtime_error("Attempting to add a TPGEdge between vertices "
                                 "not present in the TPGGraph.");
    }

    // Create the edge
    this->edges.push_back(factory->createTPGEdge(&src, &dest, prog));
    TPGEdge& newEdge = *(this->edges.back());

    // Add the edged to the Vertices
    try {
        // (May throw if an outgoing edge is added to an action)
        (*srcVertex)->addOutgoingEdge(&newEdge);
    }
    catch (std::runtime_error& e) {
        // Remove the edge before re-throwing
        this->edges.pop_back();
        throw e;
    }
    (*dstVertex)->addIncomingEdge(&newEdge);

    // return the new edge
    return newEdge;
}

const TPG::TPGEdge& TPG::TPGGraph::addNewActionEdge(
    const TPGVertex& src, const std::shared_ptr<Program::Program> prog,
    uint64_t actionClass)
{
    // Check the TPGVertex existence within the graph.
    auto srcVertex =
        std::find_if(this->vertices.begin(), this->vertices.end(),
                     [&src](TPG::TPGVertex* other) { return other == &src; });
    if (srcVertex == this->vertices.end()) {
        throw std::runtime_error(
            "Attempting to add a TPGActionEdge with a vertex "
            "not present in the TPGGraph.");
    }
    else if (dynamic_cast<TPG::TPGAction*>(*srcVertex) == nullptr) {
        throw std::runtime_error(
            "Attempting to add a TPGActionEdge with a vertex "
            "that is a team.");
    }

    // Create the edge
    this->edges.push_back(
        factory->createTPGActionEdge(&src, prog, actionClass));
    TPGEdge& newEdge = *(this->edges.back());

    (*srcVertex)->addOutgoingEdge(&newEdge);

    // Update the assessed actions of the source vertex
    (*srcVertex)->updateAssessedActions();

    // return the new edge
    return newEdge;
}

const std::list<std::unique_ptr<TPG::TPGEdge>>& TPG::TPGGraph::getEdges() const
{
    return this->edges;
}

void TPG::TPGGraph::removeEdge(const TPGEdge& edge)
{

    // Get the edge (if it is in the graph)
    auto iterator = std::find_if(this->edges.begin(), this->edges.end(),
                                 [&edge](std::unique_ptr<TPG::TPGEdge>& other) {
                                     return &edge == other.get();
                                 });

    // Disconnect the edge from the vertices
    if (iterator == this->edges.end()) {
        throw std::runtime_error(
            "Cannot erase a edge that does not belong to the graph");
    }

    if (dynamic_cast<const TPGActionEdge*>(iterator->get()) != nullptr) {
        return this->removeActionEdge(edge);
    }

    // This should not fail.
    this->findVertex(iterator->get()->getSource())
        ->removeOutgoingEdge(iterator->get());

    auto destination = iterator->get()->getDestination();

    // This should not fail.
    this->findVertex(destination)->removeIncomingEdge(iterator->get());

    // If destination is an action and should became a root, it is deleted if
    // the environment is continuous and does not use action program
    if (env.getNbContinuousActions() > 0 &&
        dynamic_cast<const TPG::TPGAction*>(destination) != nullptr &&
        destination->getIncomingEdges().size() == 0) {

        this->removeVertex(*destination);
    }

    // Remove the edge
    this->edges.erase(iterator);
}

void TPG::TPGGraph::removeActionEdge(const TPGEdge& edge)
{
    // Get the edge (if it is in the graph)
    auto iterator = std::find_if(this->edges.begin(), this->edges.end(),
                                 [&edge](std::unique_ptr<TPG::TPGEdge>& other) {
                                     return &edge == other.get();
                                 });

    // Disconnect the edge from the vertices
    if (iterator == this->edges.end()) {
        throw std::runtime_error(
            "Cannot erase a edge that does not belong to the graph");
    }

    // This should not fail.
    this->findVertex(iterator->get()->getSource())
        ->removeOutgoingEdge(iterator->get());

    // Remove the edge
    this->edges.erase(iterator);
}

const TPG::TPGEdge& TPG::TPGGraph::cloneEdge(const TPGEdge& edge)
{
    auto iterEdge = findEdge(&edge);
    if (iterEdge == this->edges.end()) {
        throw std::runtime_error(
            "Cannot duplicate an Edge not belonging to the graph.");
    }
    else if (dynamic_cast<const TPGActionEdge*>(iterEdge->get()) != nullptr) {
        const TPG::TPGActionEdge* actionEdge =
            dynamic_cast<const TPGActionEdge*>(iterEdge->get());
        return this->addNewActionEdge(
            *actionEdge->getSource(),
            iterEdge->get()->getProgramSharedPointer(),
            actionEdge->getActionClass());
    }
    else {
        return this->addNewEdge(*iterEdge->get()->getSource(),
                                *iterEdge->get()->getDestination(),
                                iterEdge->get()->getProgramSharedPointer());
    }
}

bool TPG::TPGGraph::setEdgeDestination(const TPGEdge& edge,
                                       const TPGVertex& newDest)
{
    // Find the edge and vertex
    auto newDestinationPtr = findVertex(&newDest);
    auto iterEdge = findEdge(&edge);
    if (newDestinationPtr != nullptr && iterEdge != this->edges.end()) {
        // Unregister the edge from the old destination
        const TPG::TPGVertex* oldDestination =
            iterEdge->get()->getDestination();
        auto oldDestinationPtr = findVertex(oldDestination);
        // finding the vertex should not fail. Otherwise, the exception for
        // next line would be well deserved since it means an edge in the
        // graph is connected to a vertex not in the graph.
        oldDestinationPtr->removeIncomingEdge(iterEdge->get());
        // Register the edge to the new destination
        newDestinationPtr->addIncomingEdge(iterEdge->get());
        // Set the destination
        iterEdge->get()->setDestination(newDestinationPtr);
        return true;
    }
    else {
        return false;
    }
}

bool TPG::TPGGraph::setEdgeSource(const TPGEdge& edge, const TPGVertex& newSrc)
{
    // Find the edge and vertex
    auto newSourcePtr = findVertex(&newSrc);
    auto iterEdge = findEdge(&edge);
    if (newSourcePtr != nullptr && iterEdge != this->edges.end()) {
        // Unregister the edge from the old source
        const TPG::TPGVertex* oldSrc = iterEdge->get()->getSource();
        auto oldSrcPtr = findVertex(oldSrc);
        // finding the vertex should not fail. Otherwise, the exception for
        // next line would be well deserved since it means an edge in the
        // graph is connected to a vertex not in the graph.
        oldSrcPtr->removeOutgoingEdge(iterEdge->get());
        // Register the edge to the new source
        newSourcePtr->addOutgoingEdge(iterEdge->get());
        // Set the destination
        iterEdge->get()->setSource(newSourcePtr);
        return true;
    }
    else {
        return false;
    }
}

TPG::TPGVertex* TPG::TPGGraph::findVertex(
    const TPG::TPGVertex* vertex)
{
    // Find the vertex based on the ID set
    auto vertexPtr = this->mapVertexIDs.count(vertex->getVertexID()) > 0 ?
               this->mapVertexIDs[vertex->getVertexID()] :
               nullptr;

    if (vertexPtr != nullptr && vertexPtr != vertex) {
        // The vertex ID exists but the pointer is different.
        // This means the vertex is not in the graph.
        return nullptr;
    }

    return vertexPtr;

}

std::list<std::unique_ptr<TPG::TPGEdge>>::iterator TPG::TPGGraph::findEdge(
    const TPGEdge* edge)
{
    return std::find_if(this->edges.begin(), this->edges.end(),
                        [&edge](std::unique_ptr<TPG::TPGEdge>& other) {
                            return other.get() == edge;
                        });
}

void TPG::TPGGraph::clearProgramIntrons()
{
    for (auto& edge : this->edges) {
        edge.get()->getProgram().clearIntrons();
    }
}

void TPG::TPGGraph::setActionClassEdge(const TPGEdge* edge,
                                       uint64_t newActionClass)
{
    auto it = this->findEdge(edge);

    if (it != this->edges.end()) {
        if (dynamic_cast<TPG::TPGActionEdge*>(it->get()) == nullptr) {
            throw std::runtime_error(
                "Trying to set an action class on a context edge");
        }
        // Found the edge, modify it as needed
        dynamic_cast<TPG::TPGActionEdge*>(it->get())->setActionClass(
            newActionClass);
    }
    else {
        throw std::runtime_error("Edges not in the graph.");
    }
}

void TPG::TPGGraph::updateAssessedActions(const TPG::TPGVertex* vertex)
{
    std::queue<const TPG::TPGVertex*> vertexToUpdate;
    vertexToUpdate.push(vertex);

    while (!vertexToUpdate.empty()) {
        // Get the front vertex in the queue
        auto currentVertex = vertexToUpdate.front();
        vertexToUpdate.pop();

        // Find the vertex to get the non-const reference
        auto vertexPtr = this->findVertex(currentVertex);
        if (vertexPtr != nullptr) {
            // Add the vertices leading to the current vertex to the queue
            for (auto incomingEdge : vertexPtr->getIncomingEdges()) {
                vertexToUpdate.push(incomingEdge->getSource());
            }

            // Update assessed actions for the current vertex
            vertexPtr->updateAssessedActions();
        }
        else {
            throw std::runtime_error(
                "Vertex to assess actions not in the graph.");
        }
    }
}

void TPG::TPGGraph::updateAllAssessedActions()
{

    // Launch update method for all actions.
    // All teams should be linked to actions, even not directly.
    for (auto vertex : this->vertices) {
        if (dynamic_cast<TPGAction*>(vertex) != nullptr) {
            this->updateAssessedActions(vertex);
        }
    }
}
void TPG::TPGGraph::setToBeDeleted(const TPG::TPGVertex* vertex)
{
    auto vertexPtr = this->findVertex(vertex);

    if (vertexPtr != nullptr) {
        // Found the vertex, modify it as needed
        vertexPtr->setToBeDeleted(true);
    }
    else {
        throw std::runtime_error("Action to order not in the graph.");
    }
}

void TPG::TPGGraph::orderActionEdges(const TPG::TPGAction* action)
{
    auto vertexPtr = this->findVertex(action);

    if (vertexPtr != nullptr) {
        // Found the vertex, modify it as needed
        dynamic_cast<TPG::TPGAction*>(vertexPtr)->orderActionEdges();
    }
    else {
        throw std::runtime_error("Action to order not in the graph.");
    }
}
