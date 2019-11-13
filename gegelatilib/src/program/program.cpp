#include <stdexcept>
#include <new>
#include <algorithm>
#include <set>

#include "parameter.h"
#include "program/program.h"

Program::Program::~Program() {
	while (!lines.empty()) {
		Line* line = lines.back().first;
		delete line;
		lines.pop_back();
	}
}

Program::Line& Program::Program::addNewLine() {
	return this->addNewLine(this->getNbLines());
}

Program::Line& Program::Program::addNewLine(const uint64_t idx)
{
	if (idx > this->getNbLines()) {
		throw std::out_of_range("Attempting to insert a line beyond the program end.");
	}
	// Allocate the zero-filled memory 
	Line* newLine = new Line(this->environment);
	// new line is not marked as an intron by default
	this->lines.insert(lines.begin() + idx, { newLine, false });

	return *newLine;
}

void Program::Program::removeLine(const uint64_t idx)
{
	delete this->lines.at(idx).first; // throws std::out_of_range on bad index.
	this->lines.erase(this->lines.begin() + idx);
}

void Program::Program::swapLines(const uint64_t idx0, const uint64_t idx1)
{
	if (idx0 >= this->getNbLines() || idx1 >= this->getNbLines()) {
		throw std::out_of_range("Attempting to swap a line beyond the program end.");
	}

	std::iter_swap(this->lines.begin() + idx0, this->lines.begin() + idx1);
}

const Environment& Program::Program::getEnvironment() const {
	return this->environment;
}

size_t Program::Program::getNbLines() const
{
	return this->lines.size();
}

const Program::Line& Program::Program::getLine(uint64_t index) const
{
	return *this->lines.at(index).first; // throws std::out_of_range on bad index.
}

Program::Line& Program::Program::getLine(uint64_t index)
{
	return *this->lines.at(index).first; // throws std::out_of_range on bad index.
}

void Program::Program::identifyIntrons()
{
	// Set of useful register
	std::set<uint64_t> registers;
	// Start with only register 0
	registers.insert(0);

	// Scan program lines backward
	auto backIter = this->lines.rbegin();
	while (backIter != this->lines.rend()) {
		// Check if the currentLine output is within registers
		Line* currentLine = backIter->first;
		uint64_t destinationIndex = currentLine->getDestinationIndex();
		auto destinationRegister = registers.find(destinationIndex);
		if (destinationRegister != registers.end()) {
			// The Line is useful (i.e. not an introns)
			backIter->second = false;

			// Remove the destination register from the list of useful operands
			registers.erase(*destinationRegister);

			// Add register operands to the list of usefull registers
			size_t nbOperands = this->environment.getInstructionSet().getInstruction(currentLine->getInstructionIndex()).getNbOperands();
			for (auto idxOperand = 0; idxOperand < nbOperands; idxOperand++) {
				// Is the operand a register (i.e. its index is 0)
				if (currentLine->getOperand(idxOperand).first == 0) {
					// The operand is a register, add this register to
					// the list of useful registers.
					uint64_t location = currentLine->getOperand(idxOperand).second;
					uint64_t registerIdx = location % this->environment.getNbRegisters();
					registers.insert(registerIdx);
				}
			}
		}
		else {
			// The destination of the line is not within useful registers
			// the line does not contribute to the result of the Program
			// it is an intron.
			backIter->second = true;
		}
	}
}

