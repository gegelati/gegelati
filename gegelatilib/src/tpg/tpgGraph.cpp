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

const Environment& EvoGraph::Graph::getEnvironment() const
{
    return this->env;
}

const EvoGraph::TPGFactory& EvoGraph::Graph::getFactory() const
{
    return *this->factory;
}

void EvoGraph::Graph::setNewVertexID(const EvoGraph::TPGVertex& vertex, uint64_t newID)
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

const EvoGraph::TPGTeam& EvoGraph::Graph::addNewTeam()
{
    this->vertices.insert(factory->createTPGTeam());
    return (const TPGTeam&)(*this->vertices.rbegin()->get());
}

const EvoGraph::TPGAction& EvoGraph::Graph::addNewAction(uint64_t actionID)
{
    this->vertices.insert(factory->createTPGAction(actionID));
    return (const TPGAction&)(*this->vertices.rbegin()->get());
}

size_t EvoGraph::Graph::getNbVertices() const
{
    return this->vertices.size();
}

const std::vector<const EvoGraph::TPGVertex*> EvoGraph::Graph::getVertices() const
{
    std::vector<const EvoGraph::TPGVertex*> result;
    result.reserve(this->vertices.size());

    std::transform(this->vertices.begin(), this->vertices.end(),
                   std::back_inserter(result),
                   [](const std::unique_ptr<TPGVertex>& v) { return v.get(); });

    return result;
}

uint64_t EvoGraph::Graph::getNbRootVertices() const
{
    return std::count_if(this->vertices.begin(), this->vertices.end(),
                         [](const std::unique_ptr<TPGVertex>& vertex) {
                             return vertex->getIncomingEdges().empty();
                         });
}

const std::vector<const EvoGraph::TPGAction*> EvoGraph::Graph::getRootActions() const
{
    std::vector<const EvoGraph::TPGAction*> result;
    for (auto& vertex : this->vertices) {
        if (vertex->getIncomingEdges().empty() &&
            dynamic_cast<const EvoGraph::TPGAction*>(vertex.get()) != nullptr) {
            result.push_back(dynamic_cast<const EvoGraph::TPGAction*>(vertex.get()));
        }
    }
    return result;
}

const std::vector<const EvoGraph::TPGTeam*> EvoGraph::Graph::getRootTeams() const
{
    std::vector<const EvoGraph::TPGTeam*> result;
    for (auto& vertex : this->vertices) {
        if (vertex->getIncomingEdges().empty() &&
            dynamic_cast<const EvoGraph::TPGTeam*>(vertex.get()) != nullptr) {
            result.push_back(dynamic_cast<const EvoGraph::TPGTeam*>(vertex.get()));
        }
    }
    return result;
}

const std::vector<const EvoGraph::TPGVertex*> EvoGraph::Graph::getRootVertices() const
{
    std::vector<const EvoGraph::TPGVertex*> result;
    for (auto& vertex : this->vertices) {
        if (vertex->getIncomingEdges().empty()) {
            result.push_back(vertex.get());
        }
    }
    return result;
}

bool EvoGraph::Graph::hasVertex(const TPGVertex& vertex) const
{
    return std::any_of(vertices.begin(), vertices.end(),
                       [&vertex](const std::unique_ptr<TPGVertex>& vptr) {
                           return vptr.get() == &vertex;
                       });
}

void EvoGraph::Graph::removeVertex(const TPGVertex& vertex)
{
    // Remove the vertex based on a pointer comparison.
    auto iterator = vertices.find(&vertex);
    if (iterator != this->vertices.end() && iterator->get() == &vertex) {
        // Remove all connected edges.
        // copy inEdges set for removal
        // (because iterating on the modified set is not a good idea).
        std::list<TPGEdge*> inEdgesToRemove = (*iterator)->getIncomingEdges();
        for (auto inEdge : inEdgesToRemove) {
            this->removeEdge(*inEdge);
        }
        // copy outEdges set for removal
        std::list<TPGEdge*> outEdgesToRemove = (*iterator)->getOutgoingEdges();
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

const EvoGraph::TPGVertex& EvoGraph::Graph::cloneVertex(const TPGVertex& vertex)
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
    if (dynamic_cast<const EvoGraph::TPGTeam*>(&vertex) != nullptr) {
        this->addNewTeam();
    }
    else if (dynamic_cast<const EvoGraph::TPGAction*>(&vertex) != nullptr) {
        this->addNewAction(((TPGAction&)vertex).getActionID());
    }

    // Get the new vertex
    TPGVertex* newVertex = this->vertices.rbegin()->get();

    // Copy the outgoing edges (if any).
    for (auto edge : vertex.getOutgoingEdges()) {

        if (dynamic_cast<EvoGraph::TPGActionEdge*>(edge) != nullptr) {

            // If action edge, create new action edge, else create new standard
            // edge.
            EvoGraph::TPGActionEdge* actionEdge = dynamic_cast<TPGActionEdge*>(edge);
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

void EvoGraph::Graph::setNewEdgeID(const EvoGraph::TPGEdge& edge, uint64_t newID)
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

const EvoGraph::TPGEdge& EvoGraph::Graph::addNewEdge(
    const TPGVertex& src, const TPGVertex& dest,
    const std::shared_ptr<Program::Program> prog)
{
    // Check the TPGVertex existence within the graph.
    auto srcVertex = this->vertices.find(&src);
    auto dstVertex = this->vertices.find(&dest);

    if (dstVertex == this->vertices.end() || dstVertex->get() != &dest ||
        srcVertex == this->vertices.end() || srcVertex->get() != &src) {
        throw std::runtime_error("Attempting to add a TPGEdge between vertices "
                                 "not present in the Graph.");
    }

    // Create the edge
    this->edges.insert(factory->createTPGEdge(&src, &dest, prog));
    TPGEdge& newEdge = *(this->edges.rbegin()->get());

    // Add the edged to the Vertices
    try {
        // (May throw if an outgoing edge is added to an action)
        (*srcVertex)->addOutgoingEdge(&newEdge);
    }
    catch (std::runtime_error& e) {
        // Remove the edge before re-throwing
        this->edges.erase(std::prev(this->edges.end()));
        throw e;
    }
    (*dstVertex)->addIncomingEdge(&newEdge);

    // return the new edge
    return newEdge;
}

const EvoGraph::TPGEdge& EvoGraph::Graph::addNewActionEdge(
    const TPGVertex& src, const std::shared_ptr<Program::Program> prog,
    uint64_t actionClass)
{
    // Check the TPGVertex existence within the graph.
    auto srcVertex = this->vertices.find(&src);

    if (srcVertex == this->vertices.end() || srcVertex->get() != &src) {
        throw std::runtime_error(
            "Attempting to add a TPGActionEdge with a vertex "
            "not present in the Graph.");
    }
    else if (dynamic_cast<EvoGraph::TPGAction*>(srcVertex->get()) == nullptr) {
        throw std::runtime_error(
            "Attempting to add a TPGActionEdge with a vertex "
            "that is a team.");
    }

    // Create the edge
    this->edges.insert(factory->createTPGActionEdge(&src, prog, actionClass));
    TPGEdge& newEdge = *(this->edges.rbegin()->get());

    (*srcVertex)->addOutgoingEdge(&newEdge);

    // Update the assessed actions of the source vertex
    (*srcVertex)->updateAssessedActions();

    // return the new edge
    return newEdge;
}

const std::set<std::unique_ptr<EvoGraph::TPGEdge>, UniqueLess<EvoGraph::TPGEdge>>& EvoGraph::
    Graph::getEdges() const
{
    return this->edges;
}

void EvoGraph::Graph::removeEdge(const TPGEdge& edge)
{

    // Get the edge (if it is in the graph)
    auto iterator = this->edges.find(&edge);

    // Disconnect the edge from the vertices
    if (iterator == this->edges.end() || iterator->get() != &edge) {
        throw std::runtime_error(
            "Cannot erase a edge that does not belong to the graph");
    }

    if (dynamic_cast<const TPGActionEdge*>(iterator->get()) != nullptr) {
        return this->removeActionEdge(edge);
    }

    (*this->vertices.find(iterator->get()->getSource()))
        ->removeOutgoingEdge(iterator->get());

    auto destination = iterator->get()->getDestination();
    (*this->vertices.find(destination))->removeIncomingEdge(iterator->get());

    // If destination is an action and should became a root, it is deleted if
    // the environment is continuous and does not use action program
    if (env.getNbContinuousActions() > 0 &&
        dynamic_cast<const EvoGraph::TPGAction*>(destination) != nullptr &&
        destination->getIncomingEdges().size() == 0) {

        this->removeVertex(*destination);
    }

    // Remove the edge
    this->edges.erase(iterator);
}

void EvoGraph::Graph::removeActionEdge(const TPGEdge& edge)
{
    // Get the edge (if it is in the graph)
    auto iterator = this->edges.find(&edge);

    // Disconnect the edge from the vertices
    if (iterator == this->edges.end() || iterator->get() != &edge) {
        throw std::runtime_error(
            "Cannot erase a edge that does not belong to the graph");
    }

    (*this->vertices.find(iterator->get()->getSource()))
        ->removeOutgoingEdge(iterator->get());

    // Remove the edge
    this->edges.erase(iterator);
}

const EvoGraph::TPGEdge& EvoGraph::Graph::cloneEdge(const TPGEdge& edge)
{
    auto iterEdge = this->edges.find(&edge);
    if (iterEdge == this->edges.end() || iterEdge->get() != &edge) {
        throw std::runtime_error(
            "Cannot duplicate an Edge not belonging to the graph.");
    }
    else if (dynamic_cast<const TPGActionEdge*>(iterEdge->get()) != nullptr) {
        const EvoGraph::TPGActionEdge* actionEdge =
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

bool EvoGraph::Graph::setEdgeDestination(const TPGEdge& edge,
                                       const TPGVertex& newDest)
{
    // Find the edge and vertex
    auto iterNewDestination = this->vertices.find(&newDest);
    auto iterEdge = this->edges.find(&edge);
    if (iterNewDestination != this->vertices.end() &&
        iterEdge != this->edges.end() &&
        iterNewDestination->get() == &newDest && iterEdge->get() == &edge) {
        // Unregister the edge from the old destination
        const EvoGraph::TPGVertex* oldDestination =
            iterEdge->get()->getDestination();
        auto iterOldDest = this->vertices.find(oldDestination);
        // finding the vertex should not fail. Otherwise, the exception for
        // next line would be well deserved since it means an edge in the
        // graph is connected to a vertex not in the graph.
        (*iterOldDest)->removeIncomingEdge(iterEdge->get());
        // Register the edge to the new destination
        (*iterNewDestination)->addIncomingEdge(iterEdge->get());
        // Set the destination
        iterEdge->get()->setDestination(iterNewDestination->get());
        return true;
    }
    else {
        return false;
    }
}

bool EvoGraph::Graph::setEdgeSource(const TPGEdge& edge, const TPGVertex& newSrc)
{
    // Find the edge and vertex
    auto iterNewSrc = this->vertices.find(&newSrc);
    auto iterEdge = this->edges.find(&edge);
    if (iterNewSrc != this->vertices.end() && iterEdge != this->edges.end() &&
        iterNewSrc->get() == &newSrc && iterEdge->get() == &edge) {
        // Unregister the edge from the old source
        const EvoGraph::TPGVertex* oldSrc = iterEdge->get()->getSource();
        auto iterOldSrc = this->vertices.find(oldSrc);
        // finding the vertex should not fail. Otherwise, the exception for
        // next line would be well deserved since it means an edge in the
        // graph is connected to a vertex not in the graph.
        (*iterOldSrc)->removeOutgoingEdge(iterEdge->get());
        // Register the edge to the new source
        (*iterNewSrc)->addOutgoingEdge(iterEdge->get());
        // Set the destination
        iterEdge->get()->setSource(iterNewSrc->get());
        return true;
    }
    else {
        return false;
    }
}

void EvoGraph::Graph::clearProgramIntrons()
{
    for (auto& edge : this->edges) {
        edge.get()->getProgram().clearIntrons();
    }
}

void EvoGraph::Graph::setActionClassEdge(const TPGEdge* edge,
                                       uint64_t newActionClass)
{
    auto it = this->edges.find(edge);

    if (it != this->edges.end() && it->get() == edge) {
        if (dynamic_cast<EvoGraph::TPGActionEdge*>(it->get()) == nullptr) {
            throw std::runtime_error(
                "Trying to set an action class on a context edge");
        }
        // Found the edge, modify it as needed
        dynamic_cast<EvoGraph::TPGActionEdge*>(it->get())->setActionClass(
            newActionClass);
    }
    else {
        throw std::runtime_error("Edges not in the graph.");
    }
}

void EvoGraph::Graph::updateAssessedActions(const EvoGraph::TPGVertex* vertex)
{
    std::queue<const EvoGraph::TPGVertex*> vertexToUpdate;
    vertexToUpdate.push(vertex);

    while (!vertexToUpdate.empty()) {
        // Get the front vertex in the queue
        auto currentVertex = vertexToUpdate.front();
        vertexToUpdate.pop();

        // Find the vertex to get the non-const reference
        auto it = this->vertices.find(currentVertex);
        if (it != this->vertices.end() && it->get() == currentVertex) {
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
        if (dynamic_cast<TPGAction*>(vertex.get()) != nullptr) {
            this->updateAssessedActions(vertex.get());
        }
    }
}

void EvoGraph::Graph::orderActionEdges(const EvoGraph::TPGAction* action)
{
    auto it = this->vertices.find(action);

    if (it != this->vertices.end() && it->get() == action) {
        // Found the vertex, modify it as needed
        dynamic_cast<EvoGraph::TPGAction*>(it->get())->orderActionEdges();
    }
    else {
        throw std::runtime_error("Action to order not in the graph.");
    }
}