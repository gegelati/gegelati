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

#include "evoGraph/graph.h"

EvoGraph::Graph::~Graph()
{
    clear();
}

EvoGraph::Graph& EvoGraph::Graph::operator=(Graph model)
{
    swap(*this, model);
    return *this;
}

void EvoGraph::Graph::clear()
{
    this->vertices.clear();
    this->edges.clear();
}

const EvoGraph::GraphFactory& EvoGraph::Graph::getFactory() const
{
    return *this->factory;
}

void EvoGraph::Graph::setNewVertexID(const EvoGraph::Vertex& vertex, uint64_t newID)
{
    // Check that the vertex to modify exists in the graph
    auto vertexIterator = vertices.find(&vertex);
    if (vertexIterator == this->vertices.end() ||
        vertexIterator->get() != &vertex) {
        throw std::runtime_error(
            "The vertex to modify does not exist in the Graph.");
    }

    // Check that no other vertex has the same ID
    for (const auto& vptr : vertices) {
        if (vptr.get() != &vertex && vptr->getVertexID() == newID) {
            throw std::runtime_error("Another vertex with the same ID already "
                                     "exists in the Graph.");
        }
    }

    // Modify the ID, but removed and add again the vertex for that.
    auto tmp = vertices.extract(vertexIterator);
    tmp.value()->setVertexID(newID);
    vertices.insert(std::move(tmp));
}

std::shared_ptr<const EvoGraph::Team> EvoGraph::Graph::addNewTeam(std::shared_ptr<const Algorithm::Agent> programAgent)
{
    this->vertices.insert(factory->createTeam(programAgent));
    return std::dynamic_pointer_cast<const Team>(*this->vertices.rbegin());
}

std::shared_ptr<const EvoGraph::Action> EvoGraph::Graph::addNewAction(uint64_t actionID, std::shared_ptr<const Algorithm::Agent> programAgent)
{
    this->vertices.insert(factory->createAction(actionID, programAgent));
    return std::dynamic_pointer_cast<const Action>(*this->vertices.rbegin());
}

size_t EvoGraph::Graph::getNbVertices() const
{
    return this->vertices.size();
}

const std::vector<std::shared_ptr<const EvoGraph::Vertex>> EvoGraph::Graph::getVertices() const
{
    return std::vector<std::shared_ptr<const EvoGraph::Vertex>>(
        this->vertices.begin(),
        this->vertices.end()
    );
}

uint64_t EvoGraph::Graph::getNbRootVertices() const
{
    return std::count_if(this->vertices.begin(), this->vertices.end(),
                         [](const std::shared_ptr<Vertex>& vertex) {
                             return vertex->getIncomingEdges().empty();
                         });
}

const std::vector<std::shared_ptr<const EvoGraph::Action>> EvoGraph::Graph::getRootActions() const
{
    std::vector<std::shared_ptr<const EvoGraph::Action>> result;
    for (auto& vertex : this->vertices) {
        auto castedVertex = std::dynamic_pointer_cast<const Action>(vertex);
        if (vertex->getIncomingEdges().empty() &&
            castedVertex != nullptr) {
            result.push_back(castedVertex);
        }
    }
    return result;
}

const std::vector<std::shared_ptr<const EvoGraph::Team>> EvoGraph::Graph::getRootTeams() const
{
    std::vector<std::shared_ptr<const EvoGraph::Team>> result;
    for (auto& vertex : this->vertices) {
        auto castedVertex = std::dynamic_pointer_cast<const Team>(vertex);
        if (vertex->getIncomingEdges().empty() &&
            castedVertex != nullptr) {
            result.push_back(castedVertex);
        }
    }
    return result;
}

const std::vector<std::shared_ptr<const EvoGraph::Vertex>> EvoGraph::Graph::getRootVertices() const
{
    std::vector<std::shared_ptr<const EvoGraph::Vertex>> result;
    for (auto vertex : this->vertices) {
        if (vertex->getIncomingEdges().empty()) {
            result.push_back(vertex);
        }
    }
    return result;
}

bool EvoGraph::Graph::hasVertex(const Vertex& vertex) const
{
    auto iterator = vertices.find(&vertex);
    return (iterator != this->vertices.end() && iterator->get() == &vertex);
}

void EvoGraph::Graph::removeVertex(const Vertex& vertex)
{
    // Remove the vertex based on a pointer comparison.
    auto iterator = vertices.find(&vertex);
    if (iterator != this->vertices.end() && iterator->get() == &vertex) {
        // Remove all connected edges.
        // copy inEdges set for removal
        // (because iterating on the modified set is not a good idea).
        std::list<std::shared_ptr<const EvoGraph::Edge>> inEdgesToRemove = (*iterator)->getIncomingEdges();
        for (auto inEdge : inEdgesToRemove) {
            this->removeEdge(*inEdge);
        }
        // copy outEdges set for removal
        std::list<std::shared_ptr<const EvoGraph::Edge>> outEdgesToRemove = (*iterator)->getOutgoingEdges();
        for (auto outEdge : outEdgesToRemove) {
            this->removeEdge(*outEdge);
        }
    }

    // Remove edge for action can launch again remove vertex.
    // Check again if the vertex is in the graph before deleting.
    iterator = vertices.find(&vertex);
    if (iterator != this->vertices.end() && iterator->get() == &vertex) {
        // Remove the pointer from the list.
        this->vertices.erase(iterator);
    }
}

std::shared_ptr<const EvoGraph::Vertex> EvoGraph::Graph::cloneVertex(const Vertex& vertex)
{
    // Check that the vertex to clone exists in the graph
    auto vertexIterator = vertices.find(&vertex);
    if (vertexIterator == this->vertices.end() ||
        vertexIterator->get() != &vertex) {
        throw std::runtime_error(
            "The vertex to clone does not exist in the Graph.");
    }

    // Create a new Vertex
    // (at the end of the vertices list)
    if (std::dynamic_pointer_cast<const EvoGraph::Team>(*vertexIterator) != nullptr) {
        this->addNewTeam((*vertexIterator)->getProgram());
    }
    else if (auto action = std::dynamic_pointer_cast<const EvoGraph::Action>(*vertexIterator)) {
        this->addNewAction(action->getActionID(), action->getProgram());
    }

    // Get the new vertex
    std::shared_ptr<EvoGraph::Vertex> newVertex = *this->vertices.rbegin();

    // Copy the outgoing edges (if any).
    for (auto edge : vertex.getOutgoingEdges()) {

        if (auto actionEdge = std::dynamic_pointer_cast<const ActionEdge>(edge)) {

            // If action edge, create new action edge, else create new standard
            // edge.
            this->addNewActionEdge(*newVertex,
                                   actionEdge->getProgram(),
                                   actionEdge->getActionClass());
        }
        else if (edge != nullptr) {
            this->addNewEdge(*newVertex, *(edge->getDestination()),
                             edge->getProgram());
        }
        else {
            throw std::runtime_error("Edge copied should not be a nullptr.");
        }
    }

    newVertex->updateAssessedActions();

    return newVertex;
}

void EvoGraph::Graph::setNewEdgeID(const EvoGraph::Edge& edge, uint64_t newID)
{
    // Check that the edge to modify exists in the graph
    auto edgeIterator = edges.find(&edge);
    if (edgeIterator == this->edges.end() || edgeIterator->get() != &edge) {
        throw std::runtime_error(
            "The edge to modify does not exist in the Graph.");
    }

    // Check that no other edge has the same ID
    for (const auto& eptr : edges) {
        if (eptr.get() != &edge && eptr->getEdgeID() == newID) {
            throw std::runtime_error("Another edge with the same ID already "
                                     "exists in the Graph.");
        }
    }

    // Modify the ID, but removed and add again the edge for that.
    auto tmp = edges.extract(edgeIterator);
    tmp.value()->setEdgeID(newID);
    edges.insert(std::move(tmp));
}

std::shared_ptr<const EvoGraph::Edge> EvoGraph::Graph::addNewEdge(
    const Vertex& src, const Vertex& dest,
    const std::shared_ptr<const Algorithm::Agent> prog)
{
    // Check the Vertex existence within the graph.
    auto srcVertex = this->vertices.find(&src);
    auto dstVertex = this->vertices.find(&dest);

    if (dstVertex == this->vertices.end() || dstVertex->get() != &dest ||
        srcVertex == this->vertices.end() || srcVertex->get() != &src) {
        throw std::runtime_error("Attempting to add a Edge between vertices "
                                 "not present in the Graph.");
    }

    // Create the edge
    this->edges.insert(factory->createEdge(*srcVertex, *dstVertex, prog));
    auto newEdge = this->edges.rbegin();

    // Add the edged to the Vertices
    try {
        // (May throw if an outgoing edge is added to an action)
        (*srcVertex)->addOutgoingEdge(*newEdge);
    }
    catch (std::runtime_error& e) {
        // Remove the edge before re-throwing
        this->edges.erase(std::prev(this->edges.end()));
        throw e;
    }
    (*dstVertex)->addIncomingEdge(*newEdge);

    // return the new edge
    return *newEdge;
}

std::shared_ptr<const EvoGraph::Edge> EvoGraph::Graph::addNewActionEdge(
    const Vertex& src, const std::shared_ptr<const Algorithm::Agent> prog,
    uint64_t actionClass)
{
    // Check the Vertex existence within the graph.
    auto srcVertex = this->vertices.find(&src);

    if (srcVertex == this->vertices.end() || srcVertex->get() != &src) {
        throw std::runtime_error(
            "Attempting to add a ActionEdge with a vertex "
            "not present in the Graph.");
    }
    else if (std::dynamic_pointer_cast<EvoGraph::Action>(*srcVertex) == nullptr) {
        throw std::runtime_error(
            "Attempting to add a ActionEdge with a vertex "
            "that is a team.");
    }

    // Create the edge
    this->edges.insert(factory->createActionEdge(*srcVertex, prog, actionClass));
    auto newEdge = this->edges.rbegin();

    (*srcVertex)->addOutgoingEdge(*newEdge);

    // Update the assessed actions of the source vertex
    (*srcVertex)->updateAssessedActions();

    // return the new edge
    return *newEdge;
}

const std::vector<std::shared_ptr<const EvoGraph::Edge>> EvoGraph::
    Graph::getEdges() const
{
    
    return std::vector<std::shared_ptr<const EvoGraph::Edge>>(
        this->edges.begin(),
        this->edges.end()
    );
}

void EvoGraph::Graph::removeEdge(const Edge& edge)
{

    // Get the edge (if it is in the graph)
    auto iterator = this->edges.find(&edge);

    // Disconnect the edge from the vertices
    if (iterator == this->edges.end() || iterator->get() != &edge) {
        throw std::runtime_error(
            "Cannot erase a edge that does not belong to the graph");
    }

    if (std::dynamic_pointer_cast<const ActionEdge>(*iterator) != nullptr) {
        return this->removeActionEdge(edge);
    }

    (*this->vertices.find(iterator->get()->getSource()))
        ->removeOutgoingEdge(*iterator);

    auto destination = iterator->get()->getDestination();
    (*this->vertices.find(destination))->removeIncomingEdge(*iterator);

    // Remove the edge
    this->edges.erase(iterator);
}

void EvoGraph::Graph::removeActionEdge(const Edge& edge)
{
    // Get the edge (if it is in the graph)
    auto iterator = this->edges.find(&edge);

    // Disconnect the edge from the vertices
    if (iterator == this->edges.end() || iterator->get() != &edge) {
        throw std::runtime_error(
            "Cannot erase a edge that does not belong to the graph");
    }

    (*this->vertices.find(iterator->get()->getSource()))
        ->removeOutgoingEdge(*iterator);

    // Remove the edge
    this->edges.erase(iterator);
}


void EvoGraph::Graph::setVertexProgram(const Vertex& vertex, std::shared_ptr<const Algorithm::Agent> programAgent)
{
    // Check the Vertex existence within the graph.
    auto srcVertex = this->vertices.find(&vertex);

    if (srcVertex == this->vertices.end() || srcVertex->get() != &vertex) {
        throw std::runtime_error(
            "Attempting to add a ActionEdge with a vertex "
            "not present in the Graph.");
    }

    (*srcVertex)->setProgram(programAgent);
}


std::shared_ptr<const EvoGraph::Edge> EvoGraph::Graph::cloneEdge(const Edge& edge)
{
    auto iterEdge = this->edges.find(&edge);
    if (iterEdge == this->edges.end() || iterEdge->get() != &edge) {
        throw std::runtime_error(
            "Cannot duplicate an Edge not belonging to the graph.");
    }
    else if (auto actionEdge = std::dynamic_pointer_cast<const ActionEdge>(*iterEdge)) {
        return this->addNewActionEdge(
            *actionEdge->getSource(),
            iterEdge->get()->getProgram(),
            actionEdge->getActionClass());
    }
    else {
        return this->addNewEdge(*iterEdge->get()->getSource(),
                                *iterEdge->get()->getDestination(),
                                iterEdge->get()->getProgram());
    }
}

bool EvoGraph::Graph::setEdgeDestination(const Edge& edge,
                                       const Vertex& newDest)
{
    // Find the edge and vertex
    auto iterNewDestination = this->vertices.find(&newDest);
    auto iterEdge = this->edges.find(&edge);
    if (iterNewDestination != this->vertices.end() &&
        iterEdge != this->edges.end() &&
        iterNewDestination->get() == &newDest && iterEdge->get() == &edge) {
        // Unregister the edge from the old destination
        std::shared_ptr<const EvoGraph::Vertex> oldDestination =
            iterEdge->get()->getDestination();
        auto iterOldDest = this->vertices.find(oldDestination);
        // finding the vertex should not fail. Otherwise, the exception for
        // next line would be well deserved since it means an edge in the
        // graph is connected to a vertex not in the graph.
        (*iterOldDest)->removeIncomingEdge(*iterEdge);
        // Register the edge to the new destination
        (*iterNewDestination)->addIncomingEdge(*iterEdge);
        // Set the destination
        iterEdge->get()->setDestination(*iterNewDestination);
        return true;
    }
    return false;
}

bool EvoGraph::Graph::setEdgeSource(const Edge& edge, const Vertex& newSrc)
{
    // Find the edge and vertex
    auto iterNewSrc = this->vertices.find(&newSrc);
    auto iterEdge = this->edges.find(&edge);
    if (iterNewSrc != this->vertices.end() && iterEdge != this->edges.end() &&
        iterNewSrc->get() == &newSrc && iterEdge->get() == &edge) {
        // Unregister the edge from the old source
        std::shared_ptr<const EvoGraph::Vertex> oldSrc = iterEdge->get()->getSource();
        auto iterOldSrc = this->vertices.find(oldSrc);
        // finding the vertex should not fail. Otherwise, the exception for
        // next line would be well deserved since it means an edge in the
        // graph is connected to a vertex not in the graph.
        (*iterOldSrc)->removeOutgoingEdge(*iterEdge);
        // Register the edge to the new source
        (*iterNewSrc)->addOutgoingEdge(*iterEdge);
        // Set the destination
        iterEdge->get()->setSource(*iterNewSrc);
        return true;
    }
    return false;
    
}


bool EvoGraph::Graph::setEdgeProgram(const Edge& edge, std::shared_ptr<const Algorithm::Agent> programAgent)
{
    
    auto iterEdge = this->edges.find(&edge);
    if (iterEdge != this->edges.end() && iterEdge->get() == &edge) {
        // Found the edge, modify it as needed
        iterEdge->get()->setProgram(programAgent);
        return true;
    }
    return false;
}

void EvoGraph::Graph::setActionClassEdge(std::shared_ptr<const Edge> edge,
                                       uint64_t newActionClass)
{
    auto it = this->edges.find(edge);

    if (it != this->edges.end() && *it == edge) {
        if (std::dynamic_pointer_cast<EvoGraph::ActionEdge>(*it) == nullptr) {
            throw std::runtime_error(
                "Trying to set an action class on a context edge");
        }
        // Found the edge, modify it as needed
        std::dynamic_pointer_cast<EvoGraph::ActionEdge>(*it)->setActionClass(
            newActionClass);
    }
    else {
        throw std::runtime_error("Edges not in the graph.");
    }
}

void EvoGraph::Graph::updateAssessedActions(std::shared_ptr<const EvoGraph::Vertex> vertex)
{
    std::queue<std::shared_ptr<const EvoGraph::Vertex>> vertexToUpdate;
    vertexToUpdate.push(vertex);

    while (!vertexToUpdate.empty()) {
        // Get the front vertex in the queue
        auto currentVertex = vertexToUpdate.front();
        vertexToUpdate.pop();

        // Find the vertex to get the non-const reference
        auto it = this->vertices.find(currentVertex);
        if (it != this->vertices.end() && *it == currentVertex) {
            // Add the vertices leading to the current vertex to the queue
            for (auto incomingEdge : (*it)->getIncomingEdges()) {
                vertexToUpdate.push(incomingEdge->getSource());
            }

            // Update assessed actions for the current vertex
            (*it)->updateAssessedActions();
        }
        else {
            throw std::runtime_error(
                "Vertex to assess actions not in the graph.");
        }
    }
}

void EvoGraph::Graph::updateAllAssessedActions()
{

    // Launch update method for all actions.
    // All teams should be linked to actions, even not directly.
    for (const auto& vertex : this->vertices) {
        if (std::dynamic_pointer_cast<Action>(vertex) != nullptr) {
            this->updateAssessedActions(vertex);
        }
    }
}

void EvoGraph::Graph::orderActionEdges(std::shared_ptr<const Action> action)
{
    auto it = this->vertices.find(action);

    if (it != this->vertices.end() && *it == action) {
        // Found the vertex, modify it as needed
        std::dynamic_pointer_cast<Action>(*it)->orderActionEdges();
    }
    else {
        throw std::runtime_error("Action to order not in the graph.");
    }
}