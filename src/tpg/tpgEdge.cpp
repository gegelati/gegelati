#include "tpg/tpgEdge.h"

const Program::Program& TPG::TPGEdge::getProgram() const
{
	return *this->program;
}

Program::Program& TPG::TPGEdge::getProgram()
{
	return *this->program;
}

void TPG::TPGEdge::setProgram(const std::shared_ptr<Program::Program> prog)
{
	this->program = prog;
}

const TPG::TPGVertex* TPG::TPGEdge::getSource() const
{
	return this->source;
}

void TPG::TPGEdge::setSource(TPGVertex* newSource)
{
	this->source = newSource;
}

const TPG::TPGVertex* TPG::TPGEdge::getDestination() const
{
	return this->destination;
}

void TPG::TPGEdge::setDestination(TPGVertex* newDestination)
{
	this->destination = newDestination;
}
