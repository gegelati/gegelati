#include <stdexcept>

#include "program\line.h"

uint64_t Program::Line::getDestination() const
{
	return this->destination;
}

bool Program::Line::setDestination(uint64_t dest, bool check)
{
	if (check && dest >= this->environment.getNbRegisters()) {
		return false;
	}
	this->destination = dest;
	return true;
}

uint64_t Program::Line::getInstruction() const
{
	return this->instruction;
}

bool Program::Line::setInstruction(uint64_t instr, bool check)
{
	if (check && instr >= this->environment.getNbInstructions()) {
		return false;
	}
	this->instruction = instr;
	return true;
}

Parameter Program::Line::getParameter(uint64_t idx) const
{
	if (idx >= this->environment.getMaxNbParameters()) {
		throw std::range_error("Attempting to access an non-existing Parameter.");
	}

	return this->parameters[idx];
}

void Program::Line::setParameter(const uint64_t idx, const Parameter p)
{
	if (idx >= this->environment.getMaxNbParameters()) {
		throw std::range_error("Attempting to set an non-existing Parameter.");
	}

	this->parameters[idx] = p;
}
