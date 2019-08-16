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

const std::pair<uint64_t, uint64_t>& Program::Line::getOperand(const uint64_t idx) const
{
	if (idx >= this->environment.getMaxNbOperands()) {
		throw std::range_error("Attempting to access an non-existing operand.");
	}

	return this->operands[idx];
}

bool Program::Line::setOperand(const uint64_t idx, const uint64_t dataIndex, const uint64_t location, const bool check)
{
	if (idx >= this->environment.getMaxNbOperands()) {
		throw std::range_error("Attempting to set an non-existing operand.");
	}

	if (check) {
		// Check data Index
		if (dataIndex >= this->environment.getNbDataSources()) {
			return false;
		}

		// Check location
		const bool isRegister = (dataIndex == 0);
		if (isRegister && location >= this->environment.getNbRegisters()) {
			return false;
		}

		if (!isRegister && location >= this->environment.getDataSources().at(dataIndex - 1).get().getLargestAddressSpace()) {
			return false;
		}
	}

	this->operands[idx].first = dataIndex;
	this->operands[idx].second = location;

	return true;
}
