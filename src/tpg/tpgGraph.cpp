#include <algorithm>
#include <stdexcept>

#include "tpg/tpgGraph.h"

TPG::TPGGraph::~TPGGraph()
{
	for (TPG::TPGVertex* vertex : this->vertices) {
		delete vertex;
	}
}

const TPG::TPGTeam& TPG::TPGGraph::addNewTeam() {
	this->vertices.push_back(new TPG::TPGTeam());
	return (const TPGTeam&)(*this->vertices.back());
}

const TPG::TPGAction& TPG::TPGGraph::addNewAction()
{
	this->vertices.push_back(new TPG::TPGAction());
	return (const TPGAction&)(*this->vertices.back());
}

const std::vector<const TPG::TPGVertex *> TPG::TPGGraph::getVertices() const
{
	std::vector<const TPG::TPGVertex*> result(this->vertices.size());
	std::copy(this->vertices.begin(), this->vertices.end(), result.begin());
	return result;
}

void TPG::TPGGraph::removeVertex(const TPGVertex& vertex)
{
	// Remove the vertex based on a pointer comparison.
	auto iterator = std::find_if(this->vertices.begin(), this->vertices.end(),
		[&vertex](TPG::TPGVertex* other) {
			return other == &vertex;
		});
	if (iterator != this->vertices.end()) {
		this->vertices.erase(iterator);
	}
}

TPG::TPGEdge& TPG::TPGGraph::addNewEdge(const TPGVertex& src, const TPGVertex& dest, const std::shared_ptr<Program::Program> prog)
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
