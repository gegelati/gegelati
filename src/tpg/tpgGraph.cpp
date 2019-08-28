#include "tpg/tpgGraph.h"

const TPG::TPGTeam& TPG::TPGGraph::addNewTeam() {
	TPGTeam* newTeam = new TPG::TPGTeam();
	this->vertices.push_back(std::move(*newTeam));
	return (const TPGTeam&)((this->vertices.back()));
}

const TPG::TPGAction& TPG::TPGGraph::addNewAction()
{
	TPGAction* newAction = new TPG::TPGAction();
	this->vertices.push_back(std::move(*newAction));
	return (const TPGAction&)((this->vertices.back()));
}

const std::list<TPG::TPGVertex>& TPG::TPGGraph::getVertices() const
{
	return this->vertices;
}

void TPG::TPGGraph::removeVertex(const TPGVertex& vertex)
{
	// Remove the vertex based on a pointer comparison.
	auto iterator = std::find_if(this->vertices.begin(), this->vertices.end(),
		[&vertex](TPG::TPGVertex& other) {
			return &other == &vertex; 
		});
	if (iterator != this->vertices.end()) {
		this->vertices.erase(iterator);
	}
}
