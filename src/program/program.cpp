#include <stdexcept>
#include <new>
#include <algorithm>

#include "parameter.h"
#include "program/program.h"

Program::Program::~Program() {
	while (!lines.empty()) {
		Line* line = lines.back();
		free(line);
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
	this->lines.insert(lines.begin() + idx, newLine);

	return *newLine;
}

void Program::Program::removeLine(const uint64_t idx)
{
	free(this->lines.at(idx)); // throws std::out_of_range on bad index.
	this->lines.erase(this->lines.begin() + idx);
}

void Program::Program::swapLines(const uint64_t idx0, const uint64_t idx1)
{
	if (idx0 > this->getNbLines() || idx1 > this->getNbLines()) {
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
	return *this->lines.at(index); // throws std::out_of_range on bad index.
}

Program::Line& Program::Program::getLine(uint64_t index)
{
	return *this->lines.at(index); // throws std::out_of_range on bad index.
}

