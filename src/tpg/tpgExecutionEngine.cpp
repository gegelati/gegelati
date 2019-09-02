#include "program/programExecutionEngine.h"

#include "tpg/tpgExecutionEngine.h"

double TPG::TPGExecutionEngine::evaluateEdge(const TPGEdge& edge)
{
	// Get the program
	Program::Program& prog = edge.getProgram();

	// Create the execution environment
	Program::ProgramExecutionEngine pee(prog);

	// Execute the program and returns the result.
	return pee.executeProgram();
}
