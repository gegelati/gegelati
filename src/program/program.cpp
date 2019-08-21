#include <stdexcept>
#include <new>

#include "parameter.h"
#include "program/program.h"

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
	free(this->lines.at(idx)); // throws std::out_of_range on bad index.
	this->lines.erase(this->lines.begin() + idx);
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

