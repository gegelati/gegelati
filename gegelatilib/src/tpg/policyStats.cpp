#include "tpg/policyStats.h"

void TPG::PolicyStats::setEnvironment(const Environment& env) {

	// Object needed to perform the analysis.. should be policyStats attributes
	this->environment = &env;
	this->dataSourcesAndRegisters.push_back(environment->getFakeRegisters());
	for (auto data : environment->getDataSources()) {
		dataSourcesAndRegisters.push_back(data.get());
	}
}

void TPG::PolicyStats::analyzeLine(const Program::Line* line) {
	// Count number of use of each instruction
	auto instructionIdx = line->getInstructionIndex();
	this->nbUsagePerInstruction[instructionIdx]++; // Create key if it does not exist yet.

	// Count number of access for each location
	const Instructions::Instruction& instruction = this->environment->getInstructionSet().getInstruction(instructionIdx);
	// Scan operands
	for (size_t operandIdx = 0; operandIdx < instruction.getNbOperands(); operandIdx++) {
		const std::pair<size_t, size_t>& rawOperand = line->getOperand(operandIdx);
		const std::type_info& operandType = instruction.getOperandTypes().at(operandIdx).get();
		// TODO check for code duplicate with Program & ProgramExecutionEngine factorizing code may be possible
		const Data::DataHandler& dHandler = dataSourcesAndRegisters.at(rawOperand.first).get();
		size_t scaledLocation = rawOperand.second % dHandler.getAddressSpace(operandType); // scaling.. should not be duplicate code.
		// Get list of accessed addresses
		std::vector<size_t> accessedLocations = dHandler.getAddressesAccessed(operandType, scaledLocation);
		// Fill attribute
		for (size_t accessedLocation : accessedLocations) {
			this->nbUsagePerDataLocation[{rawOperand.first, accessedLocation}]++; // create key if it does not exists.
		}
	}
}

void TPG::PolicyStats::analyzeProgram(const Program::Program* prog)
{
	// Check if the Program was already analyzed
	auto programIterator = this->nbUsePerProgram.find(prog);
	if (programIterator != this->nbUsePerProgram.end()) {
		// Increment the number of use of this Program.
		programIterator->second++;
		return;
	}

	// Else, this is a new program: analyze it
	this->nbUsePerProgram.emplace(prog, 1);

	// Count the number of lines
	this->nbLinesPerProgram.push_back(prog->getNbLines());

	// Count the number of intron lines
	size_t nbIntronLines = 0;
	for (auto lineIdx = 0; lineIdx < prog->getNbLines(); lineIdx++) {
		if (!prog->isIntron(lineIdx)) {
			const Program::Line& line = prog->getLine(lineIdx);
			this->analyzeLine(&line);
		}
		else {
			nbIntronLines++;
		}
	}
	this->nbIntronPerProgram.push_back(nbIntronLines);
}