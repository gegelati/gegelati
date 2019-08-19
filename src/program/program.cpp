#include <stdexcept>
#include <new>

#include "parameter.h"
#include "program/program.h"

const size_t Program::Program::computeLineSize(const Environment& env)
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

Program::Program::~Program() {
	while (!lines.empty()) {
		Line* line = lines.back();
		free(line);
		lines.pop_back();
	}
}

Program::Line& Program::Program::addNewLine()
{
	// Allocate the zero-filled memory 
	Line* newLine = new Line(this->environment);
	this->lines.push_back(newLine);

	return *newLine;
}

void Program::Program::removeLine(const uint64_t idx)
{
	free(this->lines.at(idx)); // throws std::range_error on bad index.
	this->lines.erase(this->lines.begin() + idx);
}

size_t Program::Program::getNbLines() const
{
	return this->lines.size();
}

const Program::Line& Program::Program::getLine(uint64_t index) const
{
	if (index >= this->lines.size()) {
		throw std::range_error("Accessing line outside of a Program.");
	}

	return *this->lines.at(index);
}

Program::Line& Program::Program::getLine(uint64_t index)
{
	if (index >= this->lines.size()) {
		throw std::range_error("Accessing line outside of a Program.");
	}

	return *this->lines.at(index);
}

