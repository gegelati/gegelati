#include "tpg/tpgAction.h"
#include <stdexcept>

void TPG::TPGAction::addOutgoingEdge(TPGEdge* edge)
{
	throw std::runtime_error("Cannot add an outgoing edge to an Action vertex.");
}