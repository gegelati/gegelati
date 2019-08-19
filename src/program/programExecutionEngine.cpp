#include "program/line.h"
#include "program/programExecutionEngine.h"

const Program::Line& Program::ProgramExecutionEngine::getCurrentLine() const
{
	return this->program.getLine(this->programCounter);
}

const Instructions::Instruction& Program::ProgramExecutionEngine::getCurrentInstruction() const
{
	const Line& currentLine = this->getCurrentLine(); // throw std::out_of_range if the program counter is too large.
	uint64_t instructionIndex = currentLine.getInstruction();
	return this->program.getEnvironment().getInstructionSet().getInstruction(instructionIndex); // throw std::out_of_range if the index of the line is too large.
}
