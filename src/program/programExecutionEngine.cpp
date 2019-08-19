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

const void Program::ProgramExecutionEngine::fetchOperands(std::vector<std::reference_wrapper<const SupportedType>>& operands) const
{
	const Instructions::Instruction& instruction = this->getCurrentInstruction(); // throw std::out_of_range
	const Line& line = this->getCurrentLine();

	// Get as many operands as required by the instruction.
	for (uint64_t i = 0; i < instruction.getNbOperands(); i++) {
		const std::pair<uint64_t, uint64_t>& operandIndexes = line.getOperand(i);
		const DataHandlers::DataHandler& dataSource = this->dataSources.at(operandIndexes.first); // Throws std::out_of_range
		const SupportedType& data = dataSource.getDataAt(instruction.getOperandTypes().at(i).get(), operandIndexes.second); // throw out of range and invalid argument
		operands.push_back(data);
	}
}
