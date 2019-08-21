#include <stdexcept>

#include "program/line.h"

const size_t Program::Line::computeLineSize(const Environment& env)
{
	// $ceil(log2(n)) + ceil(log2(i)) + m * (ceil(log2(nb_{ src })) + ceil(log2(largestAddressSpace)) + p * sizeof(Param)_{inByte} * 8$
	const size_t n = env.getNbRegisters();

	const size_t i = env.getNbInstructions();

	const size_t m = env.getMaxNbOperands();

	const size_t nbSrc = env.getNbDataSources();

	const size_t largestAddressSpace = env.getLargestAddressSpace();

	const size_t p = env.getMaxNbParameters();

	// Add some checks on values. Only p can be null for a valid program. nbSrc 
	// cannot be 1, as it would mean an environment with only registers.
	if (n == 0 || i == 0 || m == 0 || nbSrc <= 1 || largestAddressSpace == 0) {
		throw std::domain_error("Environment given to the computeLineSize is invalid for building a program." \
			"It is parameterized with no or only registers, contains no Instruction, Instruction" \
			" with no operands, no DataHandler or DataHandler with no addressable Space.");
	}

	return (size_t)(ceill(log2l((long double)n)) + ceill(log2l((long double)i))
		+ m * (ceill(log2l((long double)nbSrc) + ceill(log2l((long double)largestAddressSpace))))
		+ p * sizeof(Parameter) * 8);
}

const Environment& Program::Line::getEnvironment() const
{
	return this->environment;
}

uint64_t Program::Line::getDestinationIndex() const
{
	return this->destinationIndex;
}

bool Program::Line::setDestinationIndex(uint64_t dest, bool check)
{
	if (check && dest >= this->environment.getNbRegisters()) {
		return false;
	}
	this->destinationIndex = dest;
	return true;
}

uint64_t Program::Line::getInstructionIndex() const
{
	return this->instructionIndex;
}

bool Program::Line::setInstructionIndex(uint64_t instr, bool check)
{
	if (check && instr >= this->environment.getNbInstructions()) {
		return false;
	}
	this->instructionIndex = instr;
	return true;
}

const Parameter& Program::Line::getParameter(uint64_t idx) const
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
		if (location >= this->environment.getLargestAddressSpace()) {
			return false;
		}
	}

	this->operands[idx].first = dataIndex;
	this->operands[idx].second = location;

	return true;
}
