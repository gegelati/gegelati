#include <algorithm>

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
		if (std::find(this->incomingEdges.begin(), this->incomingEdges.end(), edge) == this->incomingEdges.end()) {
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
		if (std::find(this->outgoingEdges.begin(), this->outgoingEdges.end(), edge) == this->outgoingEdges.end()) {
			this->outgoingEdges.push_back(edge);
		}
	}
}

void TPG::TPGVertex::removeOutgoingEdge(TPG::TPGEdge* edge)
{
	this->outgoingEdges.remove(edge);
}
