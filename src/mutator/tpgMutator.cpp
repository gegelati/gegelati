#include <stdexcept>
#include <vector>

#include "tpg/tpgAction.h"

#include "mutator/programMutator.h"
#include "mutator/tpgMutator.h"


/*
* \brief Initialize a random TPGGraph.
*
* Following Stephen Kelly's PhD Thesis, the created TPGGraph will
* contain:
*-Exactly nbAction TPGAction vertices.
* -Exactly nbAction TPGTeam vertices
* -Exactly 2 * nbAction Programs
* -Between 2 and maxInitOutgoingEdges TPGEdge per TPGTeam, where
* -Each TPGEdge connects a TPGTeam with a TPGAction.
* -Each TPGTeam is connected to a TPGAction at most once.
* -Each TPGTeam is connected to at least 2 distinct TPGAction
* -Each Program is used at most once per TPGTeam.
* -Each Program always leads to the same TPGAction.
* -Each Program is approximately used the same number of time.
* Hence, the maxInitOutgoingEdges value can not be greater than nbAction.
*
*\throw std::runtime_error if maxInitOutgoingEdges exceeds nbAction.
* Or if nbAction is smaller than 1.
*/
void Mutator::TPGMutator::initRandomTPG(TPG::TPGGraph& graph, size_t nbAction, size_t maxInitOutgoingEdges, size_t maxProgramSize)
{
	if (maxInitOutgoingEdges > nbAction) {
		throw std::runtime_error("Maximum initial number of outgoing edges cannot exceed the number of action");
	}
	if (nbAction < 2) {
		throw std::runtime_error("A TPG with a single action makes no sense.");
	}
	// Create teams, programs and Actions
	std::vector<const TPG::TPGAction*> actions;
	std::vector<const TPG::TPGTeam*> teams;
	std::vector<std::shared_ptr<Program::Program>> programs;
	for (size_t i = 0; i < nbAction; i++) {
		actions.push_back(&(graph.addNewAction(i)));
		teams.push_back(&(graph.addNewTeam()));
	}
	for (size_t i = 0; i < 2 * nbAction; i++) {
		programs.emplace_back(new Program::Program(graph.getEnvironment()));
		// RandomInit the Programs
		Mutator::ProgramMutator::initRandomProgram(*programs.back(), maxProgramSize);
	}

	// Connect each team with two distinct actions, through two distinct programs
	// Association here are determinists since randomness would uselessly 
	// complicate the code while bringing no real value since anyway, Programs
	// have been initialized randomly.
	for (size_t i = 0; i < 2 * nbAction; i++) {
		graph.addNewEdge(*teams.at(i / 2), *actions.at(((i / 2) + (i % 2)) % nbAction), programs.at(i));
	}

	//TBC
}
