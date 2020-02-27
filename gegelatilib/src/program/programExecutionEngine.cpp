#include "program/line.h"
#include "program/programExecutionEngine.h"

void Program::ProgramExecutionEngine::setProgram(const Program& prog) {
	// Check dataSource are similar in all point to the program environment
	// '-1' on this->dataSources is to ignore registers
	if (this->dataSourcesAndRegisters.size() - 1 != prog.getEnvironment().getDataSources().size()) {
		throw std::runtime_error("Data sources characteristics for Program Execution differ from Program reference Environment.");
	}
	for (size_t i = 0; i < this->dataSourcesAndRegisters.size() - 1; i++) {
		// check data source characteristics
		auto& iDataSrc = this->dataSourcesAndRegisters.at(i + (size_t)1).get();
		auto& envDataSrc = prog.getEnvironment().getDataSources().at(i).get();
		// Assume that dataSource must be (at least) a copy of each other to simplify the comparison
		// This is characterise by the two data sources having the same id
		if (iDataSrc.getId() != envDataSrc.getId()) {
			throw std::runtime_error("Data sources characteristics for Program Execution differ from Program reference Environment.");
			// If this pose a problem one day, an additional more 
			// complex check could be used as a last resort when ids 
			// of DataHandlers are different: checking equality of the 
			// lists of provided data types and the equality address 
			// space size for each data type.
		}
	}

	// Set the program
	this->program = &prog;

	// Reset Registers (in case it is not done when they are constructed)
	this->registers.resetData();

	// Reset the counters
	this->programCounter = 0;
}

const std::vector<std::reference_wrapper<const Data::DataHandler>>& Program::ProgramExecutionEngine::getDataSources() const
{
	return this->dataSources;
}

const bool Program::ProgramExecutionEngine::next()
{
	// While the next line is an intron
	// increment the program counter.
	do {
		this->programCounter++;
	} while (this->programCounter < this->program->getNbLines() && this->program->isIntron(this->programCounter));
	return this->programCounter < this->program->getNbLines();
}

const Program::Line& Program::ProgramExecutionEngine::getCurrentLine() const
{
	return this->program->getLine(this->programCounter);
}

uint64_t Program::ProgramExecutionEngine::scaleLocation(const uint64_t rawLocation, const Data::DataHandler& dataHandler, const std::type_info& type) const
{
	return rawLocation % dataHandler.getAddressSpace(type);
}

const Instructions::Instruction& Program::ProgramExecutionEngine::getCurrentInstruction() const
{
	const Line& currentLine = this->getCurrentLine(); // throw std::out_of_range if the program counter is too large.
	uint64_t instructionIndex = currentLine.getInstructionIndex();
	return this->program->getEnvironment().getInstructionSet().getInstruction(instructionIndex); // throw std::out_of_range if the index of the line is too large.
}

const void Program::ProgramExecutionEngine::fetchCurrentOperands(std::vector<std::shared_ptr<const Data::SupportedType>>& operands) const
{
	const Line& line = this->getCurrentLine(); // throw std::out_of_range
	const Instructions::Instruction& instruction = this->getCurrentInstruction(); // throw std::out_of_range

	// Get as many operands as required by the instruction.
	for (uint64_t i = 0; i < instruction.getNbOperands(); i++) {
		const std::pair<uint64_t, uint64_t>& operandIndexes = line.getOperand(i);
		const Data::DataHandler& dataSource = this->dataSourcesAndRegisters.at(operandIndexes.first); // Throws std::out_of_range
		const std::type_info& operandType = instruction.getOperandTypes().at(i).get();
		const uint64_t operandLocation = this->scaleLocation(operandIndexes.second, dataSource, operandType);
		const std::shared_ptr<const Data::SupportedType> data = dataSource.getDataAt(operandType, operandLocation);
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
	std::vector<std::shared_ptr<const Data::SupportedType>> operands;
	std::vector<std::reference_wrapper<const Parameter>> parameters;

	// Get everything needed (may throw)
	const Line& line = this->getCurrentLine();
	const Instructions::Instruction& instruction = this->getCurrentInstruction();
	this->fetchCurrentOperands(operands);
	this->fetchCurrentParameters(parameters);

	double result = instruction.execute(parameters, operands);

	this->registers.setDataAt(typeid(Data::PrimitiveType<double>), line.getDestinationIndex(), result);
}

double Program::ProgramExecutionEngine::executeProgram(const bool ignoreException)
{
	// Reset registers and programCounter
	this->registers.resetData();
	this->programCounter = 0;

	// Iterate over the lines of the Program
	bool hasNext = this->program->getNbLines() > 0;

	// Skip first lines if they are introns.
	if (hasNext && this->program->isIntron(0)) {
		hasNext = this->next();
	}

	// Execute useful lines
	while (hasNext) {

		try {
			// Execute the current line
			this->executeCurrentLine();
		}
		catch (std::out_of_range e) {
			if (!ignoreException) {
				throw; // rethrow
			}
		}

		// Increment the programCounter.
		hasNext = this->next();
	};

	// Returns the 0-indexed register. 
	// cast to primitiveType<double> to enable cast to double.
	return *std::dynamic_pointer_cast<const Data::PrimitiveType<double>>(this->registers.getDataAt(typeid(Data::PrimitiveType<double>), 0));
}
