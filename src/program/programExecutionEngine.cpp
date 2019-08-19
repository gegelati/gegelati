#include "program/line.h"
#include "program/programExecutionEngine.h"

const bool Program::ProgramExecutionEngine::next()
{
	this->programCounter++;
	return this->programCounter < this->program.getNbLines();
}

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

const void Program::ProgramExecutionEngine::fetchCurrentOperands(std::vector<std::reference_wrapper<const SupportedType>>& operands) const
{
	const Line& line = this->getCurrentLine(); // throw std::out_of_range
	const Instructions::Instruction& instruction = this->getCurrentInstruction(); // throw std::out_of_range

	// Get as many operands as required by the instruction.
	for (uint64_t i = 0; i < instruction.getNbOperands(); i++) {
		const std::pair<uint64_t, uint64_t>& operandIndexes = line.getOperand(i);
		const DataHandlers::DataHandler& dataSource = this->dataSources.at(operandIndexes.first); // Throws std::out_of_range
		const SupportedType& data = dataSource.getDataAt(instruction.getOperandTypes().at(i).get(), operandIndexes.second); // throw out of range and invalid argument
		operands.push_back(data);
	}
}

const void Program::ProgramExecutionEngine::fetchCurrentParameters(std::vector<std::reference_wrapper<const Parameter>>& parameters) const
{
	const Line& line = this->getCurrentLine(); // throw std::out_of_range
	const Instructions::Instruction& instruction = this->getCurrentInstruction(); // throw std::out_of_range

	for (uint64_t i = 0; i < instruction.getNbParameters(); i++) {
		const Parameter& p = line.getParameter(i); // throw std::out_of_range
		parameters.push_back(p);
	}
}

void Program::ProgramExecutionEngine::executeCurrentLine()
{
	std::vector<std::reference_wrapper<const SupportedType>> operands;
	std::vector<std::reference_wrapper<const Parameter>> parameters;

	// Get everything needed (may throw)
	const Line& line = this->getCurrentLine();
	const Instructions::Instruction& instruction = this->getCurrentInstruction();
	this->fetchCurrentOperands(operands);
	this->fetchCurrentParameters(parameters);

	double result = instruction.execute(parameters, operands);

	this->registers.setDataAt(typeid(PrimitiveType<double>), line.getDestination(), result);
}

double Program::ProgramExecutionEngine::executeProgram(const bool ignoreException)
{
	// Reset registers and programCounter
	this->registers.resetData();
	this->programCounter = 0;

	// Iterate over the lines of the Program
	bool hasNext = this->programCounter < this->program.getNbLines();
	while (hasNext) {

		// Execute the current line
		this->executeCurrentLine();

		// Increment the programCounter.
		hasNext = this->next();
	};

	// Returns the 0-indexed register. 
	// cast to primitiveType<double> to enable cast to double.
	return (const PrimitiveType<double>&)this->registers.getDataAt(typeid(PrimitiveType<double>), 0);
}
