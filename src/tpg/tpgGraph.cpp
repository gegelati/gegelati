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

const TPG::TPGTeam& TPG::TPGGraph::addNewTeam() {
	this->vertices.push_back(new TPG::TPGTeam());
	return (const TPGTeam&)(*this->vertices.back());
}

const TPG::TPGAction& TPG::TPGGraph::addNewAction(uint64_t actionID)
{
	this->vertices.push_back(new TPG::TPGAction(actionID));
	return (const TPGAction&)(*this->vertices.back());
}

const std::vector<const TPG::TPGVertex*> TPG::TPGGraph::getVertices() const
{
	std::vector<const TPG::TPGVertex*> result(this->vertices.size());
	std::copy(this->vertices.begin(), this->vertices.end(), result.begin());
	return result;
}

const std::vector<const TPG::TPGVertex*> TPG::TPGGraph::getRootVertices() const
{
	std::vector<const TPG::TPGVertex*> result;
	std::copy_if(this->vertices.begin(), this->vertices.end(), std::back_inserter(result),
		[](TPGVertex* vertex) {
			return vertex->getIncomingEdges().size() == 0;
		});
	return result;
}

void TPG::TPGGraph::removeVertex(const TPGVertex& vertex)
{
	// Remove the vertex based on a pointer comparison.
	auto iterator = this->findVertex(&vertex);
	if (iterator != this->vertices.end()) {
		// Remove all connected edges.
		// copy inEdges set for removal 
		// (because iterating on the modified set is not a good idea). 
		std::set<TPGEdge*> inEdgesToRemove = (*iterator)->getIncomingEdges();
		for (auto inEdge : inEdgesToRemove) {
			this->removeEdge(*inEdge);
		}
		// copy outEdges set for removal 
		std::set<TPGEdge*> outEdgesToRemove = (*iterator)->getOutgoingEdges();
		for (auto outEdge : outEdgesToRemove) {
			this->removeEdge(*outEdge);
		}

		// Free the memory of the vertex
		delete* iterator;
		// Remove the pointer from the list.
		this->vertices.erase(iterator);
	}
}

const TPG::TPGVertex& TPG::TPGGraph::cloneVertex(const TPGVertex& vertex)
{
	// Check that the vertex to clone exists in the graph
	auto vertexIterator = this->findVertex(&vertex);
	if (vertexIterator == this->vertices.end()) {
		throw std::runtime_error("The vertex to clone does not exist in the TPGGraph.");
	}

	// Create a new Vertex
	// (at the end of the vertices list)
	if (typeid(vertex) == typeid(TPGTeam)) {
		this->addNewTeam();
	}
	else if (typeid(vertex) == typeid(TPGAction)) {
		this->addNewAction(((TPGAction&)vertex).getActionID());
	}

	// Get the new vertex
	TPGVertex* newVertex = this->vertices.back();

	// Copy the outgoing edges (if any).
	for (auto edge : vertex.getOutgoingEdges()) {
		this->addNewEdge(*newVertex, *(edge->getDestination()), edge->getProgramSharedPointer());
	}

	return *newVertex;
}

const TPG::TPGEdge& TPG::TPGGraph::addNewEdge(const TPGVertex& src, const TPGVertex& dest, const std::shared_ptr<Program::Program> prog)
{
	// Check the TPGVertex existence within the graph.
	auto srcVertex = std::find_if(this->vertices.begin(), this->vertices.end(),
		[&src](TPG::TPGVertex* other) {
			return other == &src;
		});
	auto dstVertex = std::find_if(this->vertices.begin(), this->vertices.end(),
		[&dest](TPG::TPGVertex* other) {
			return other == &dest;
		});
	if (dstVertex == this->vertices.end() || srcVertex == this->vertices.end()) {
		throw std::runtime_error("Attempting to add a TPGEdge between vertices not present in the TPGGraph.");
	}

	// Create the edge
	TPGEdge& newEdge = this->edges.emplace_back(&src, &dest, prog);


	// Add the edged to the Vertices
	try {
		// (May throw if an outgoing edge is added to an action)
		(*srcVertex)->addOutgoingEdge(&newEdge);
	}
	catch (std::runtime_error e) {
		// Remove the edge before re-throwing
		this->edges.pop_back();
		throw e;
	}
	(*dstVertex)->addIncomingEdge(&newEdge);

	// return the new edge
	return newEdge;
}

const std::list<TPG::TPGEdge>& TPG::TPGGraph::getEdges() const
{
	return this->edges;
}

void TPG::TPGGraph::removeEdge(const TPGEdge& edge)
{
	// Get the edge (if it is in the graph)
	auto iterator = std::find_if(this->edges.begin(), this->edges.end(),
		[&edge](TPG::TPGEdge& other) {
			return &edge == &other;
		});
	// Disconnect the edge from the vertices
	if (iterator != this->edges.end()) {
		(*this->findVertex(iterator->getSource()))->removeOutgoingEdge(&(*iterator));
		(*this->findVertex(iterator->getDestination()))->removeIncomingEdge(&(*iterator));
	}
	// Remove the edge
	this->edges.erase(iterator);
}

std::list<TPG::TPGVertex*>::iterator TPG::TPGGraph::findVertex(const TPG::TPGVertex* vertex) {
	return std::find(this->vertices.begin(), this->vertices.end(), vertex);
}
