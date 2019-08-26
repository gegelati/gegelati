#include "tpg/tpgVertex.h"

const std::set<TPG::TPGEdge*>& TPG::TPGVertex::getIncomingEdges() const
{
	return this->incomingEdges;
}

const std::set<TPG::TPGEdge*>& TPG::TPGVertex::getOutgoingEdges() const
{
	return this->outgoingEdges;
}

void TPG::TPGVertex::addIncomingEdge(TPG::TPGEdge* edge)
{
	// Do nothing on NULL pointer
	if (edge != NULL) {
		this->incomingEdges.insert(edge);
	}
}

void TPG::TPGVertex::addOutgoingEdge(TPG::TPGEdge* edge)
{
	// Do nothing on NULL pointer
	if (edge != NULL) {
		this->outgoingEdges.insert(edge);
	}
}
