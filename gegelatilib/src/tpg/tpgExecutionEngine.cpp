#include <set>
#include <vector>
#include <algorithm>

#include "tpg/tpgEdge.h"
#include "program/programExecutionEngine.h"

#include "tpg/tpgExecutionEngine.h"

void TPG::TPGExecutionEngine::setArchive(Archive* newArchive)
{
	this->archive = newArchive;
}

double TPG::TPGExecutionEngine::evaluateEdge(const TPGEdge& edge)
{
	// Get the program
	Program::Program& prog = edge.getProgram();

	// Set the progExecutionEngine to the program
	this->progExecutionEngine.setProgram(prog);

	// Execute the program.
	double result = this->progExecutionEngine.executeProgram();

	// Put the result in the archive before returning it.
	if (this->archive != NULL) {
		this->archive->addRecording(&prog, progExecutionEngine.getDataSources(), result);
	}

	return result;
}

const TPG::TPGEdge& TPG::TPGExecutionEngine::evaluateTeam(const TPGTeam& team, const std::vector<const TPGVertex*>& excluded)
{
	// Copy outgoing edge list 
	std::list<TPG::TPGEdge*> outgoingEdges = team.getOutgoingEdges();

	// Remove all edges leading to excluded TPGTeam
	std::list<TPG::TPGEdge*>::iterator iter = outgoingEdges.begin();
	while (iter != outgoingEdges.end()) {
		if (std::find(excluded.begin(), excluded.end(), (*iter)->getDestination()) != excluded.end()) {
			outgoingEdges.erase(iter++);
		}
		else {
			iter++;
		}
	}

	// Throw an error if no edge remains
	if (outgoingEdges.size() == 0) {
		// This should not happen in a correctly constructed TPG, since every team
		// should be connected to at least one action, thus eventually breaking
		// any potential cycles.
		throw std::runtime_error("No outgoing edge to evaluate in the TPGTeam.");
	}

	// Evaluate all remaining TPGEdge
	// First
	TPGEdge* bestEdge = *outgoingEdges.begin();
	double bestBid = this->evaluateEdge(*bestEdge);
	// Others
	for (auto iter = ++outgoingEdges.begin(); iter != outgoingEdges.end(); iter++) {
		TPGEdge* edge = *iter;
		double bid = this->evaluateEdge(*edge);
		if (bid >= bestBid) {
			bestEdge = edge;
			bestBid = bid;
		}
	}

	return *bestEdge;
}

const std::vector<const TPG::TPGVertex*> TPG::TPGExecutionEngine::executeFromRoot(const TPGVertex& root)
{
	const TPGVertex* currentVertex = &root;

	std::vector<const TPGVertex*> visitedVertices;
	visitedVertices.push_back(currentVertex);

	// Browse the TPG until a TPGAction is reached.
	while (typeid(*currentVertex) == typeid(TPG::TPGTeam)) {
		// Get the next edge
		const TPGEdge& edge = this->evaluateTeam(*(TPGTeam*)currentVertex, visitedVertices);
		// update currentVertex and backup in visitedVertex.
		currentVertex = edge.getDestination();
		visitedVertices.push_back(currentVertex);
	}

	return visitedVertices;
}
