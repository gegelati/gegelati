#include <stdexcept>
#include <set>

#include "environment.h"
#include "mutator/rng.h"
#include "mutator/lineMutator.h"

/**
* \brief Function to initialize a single operand of a Program::Line.
*
* This function uses the Mutator::RNG to set (if possible) the nth operand
* pair of a given Line to a valid value according to the provided Environment
* and the selected Instruction.
*
* \param[in] instruction the selected Instruction for this line.
* \param[in,out] line the Program::Line to initialize.
* \param[in] operandIdx the index of the operand of the Line to initialize.
* \param[in] initOperandDataSource should the operand data source be (re-)initialized?
* \param[in] initOperandLocation should the operand location be (re)initialized?
* \param[in] forceChange should the value be mandatorily new?
* \return true if the operand was successfully initialized, false if no valid
* data source could be found for this Instruction and operandIdx couple.
*/
static bool initRandomCorrectLineOperand(const Instructions::Instruction& instruction, Program::Line& line, const uint64_t& operandIdx,
	const bool initOperandDataSource, const bool initOperandLocation, const bool forceChange)
{
	const Environment& env = line.getEnvironment();
	uint64_t operandDataSourceIndex = line.getOperand(operandIdx).first;
	bool operandFound = !initOperandDataSource;

	// Is the operand constrained in type?
	if (initOperandDataSource && operandIdx < instruction.getNbOperands()) {
		// Select an operand 
		// The type of operand needed
		const std::type_info& operandType = instruction.getOperandTypes().at(operandIdx).get();

		// keep a record of tested indexes
		std::set<uint64_t> operandDataSourceIndexes;
		if (forceChange) {
			operandDataSourceIndexes.insert(operandDataSourceIndex);
		}


		while (!operandFound && operandDataSourceIndexes.size() < env.getNbDataSources()) {
			// Select an operandDataSourceIndex
			operandDataSourceIndex = Mutator::RNG::getUnsignedInt64(0, (env.getNbDataSources() - 1) - operandDataSourceIndexes.size());
			// Correct the index with the number of already tested ones inferior to it.
			// This works because the set is ordered
			std::for_each(operandDataSourceIndexes.begin(), operandDataSourceIndexes.end(),
				[&operandDataSourceIndex](uint64_t index) { if (index <= operandDataSourceIndex) operandDataSourceIndex++; });
			// Add the index to the set
			operandDataSourceIndexes.insert(operandDataSourceIndex);

			// check if the selected dataSource can provide the type requested by the instruction
			if (operandDataSourceIndex == 0) {
				// Data Source is the registers
				if (operandType == typeid(PrimitiveType<double>)) {
					operandFound = true;
				}
			}
			else {
				// Data source is a dataHandler
				if (env.getDataSources().at(operandDataSourceIndex - 1).get().canHandle(operandType)) {
					operandFound = true;
				}
			}
		}

	}
	else if (initOperandDataSource) {
		// The operand is not constrained in type
		operandFound = true;
		// Select a location
		operandDataSourceIndex = Mutator::RNG::getUnsignedInt64(0, env.getNbDataSources() - 1 - ((forceChange) ? 1 : 0));
		if (forceChange && operandDataSourceIndex >= line.getOperand(operandIdx).first) {
			operandDataSourceIndex += 1;
		}
	}

	// The data source can provide the required data type
	uint64_t operandLocation = line.getOperand(operandIdx).second;
	if (operandFound && initOperandLocation) {
		// Select a location
		operandLocation = Mutator::RNG::getUnsignedInt64(0, env.getLargestAddressSpace() - 1 - ((forceChange) ? 1 : 0));
		if (forceChange && operandLocation >= line.getOperand(operandIdx).second) {
			operandLocation += 1;
		}
	}

	// set line operand info
	if (operandFound) {
		line.setOperand(operandIdx, operandDataSourceIndex, operandLocation);
	}

	return operandFound;
}

void Mutator::LineMutator::initRandomCorrectLine(Program::Line& line)
{
	const Environment& env = line.getEnvironment();

	// Select and set a destinationIndex. (can not fail)
	uint64_t destinationIndex = RNG::getUnsignedInt64(0, env.getNbRegisters() - 1);
	line.setDestinationIndex(destinationIndex); // Should never throw.. but I did not deactivate the check anyway.

	// Select and set all parameter values. (can not fail)
	for (uint64_t paramIdx = 0; paramIdx < env.getMaxNbParameters(); paramIdx++) {
		Parameter param((int16_t)(Mutator::RNG::getUnsignedInt64(0, ((int64_t)PARAM_INT_MAX - (int64_t)PARAM_INT_MIN)) + (int64_t)PARAM_INT_MIN));
		line.setParameter(paramIdx, param);
	}

	// Select an instruction.
	uint64_t instructionIndex = RNG::getUnsignedInt64(0, (env.getNbInstructions() - 1));
	// Get the instruction
	const Instructions::Instruction& instruction = env.getInstructionSet().getInstruction(instructionIndex);
	// Set the instructionIndex
	line.setInstructionIndex(instructionIndex); // Should never throw.. but I did not deactivate the check anyway.

	// Select operands needed by the instruction
	uint64_t operandIdx = 0;
	for (; operandIdx < env.getMaxNbOperands(); operandIdx++) {

		// Check if all operands were tested (and none were valid)
		initRandomCorrectLineOperand(instruction, line, operandIdx, true, true, false);

		// This operation can (no longer) fail since commit abd7cd since 
		// all Instruction are vetted when building the Environment
	}
}

void Mutator::LineMutator::alterCorrectLine(Program::Line& line)
{
	// Generate a random int to select the modified part of the line
	const LineSize lineSize = line.getEnvironment().getLineSize();
	uint64_t selectedBit = Mutator::RNG::getUnsignedInt64(0, lineSize - 1);

	// Find the selected part
	bool checkValidity = false;
	if (selectedBit < lineSize.nbInstructionBits) {
		// InstructionIndex
		// Select a random Instruction (different from the current one)
		const uint64_t currentInstructionIndex = line.getInstructionIndex();
		uint64_t newInstructionIndex = RNG::getUnsignedInt64(0, line.getEnvironment().getNbInstructions() - 2);
		newInstructionIndex += (newInstructionIndex >= currentInstructionIndex) ? 1 : 0;
		line.setInstructionIndex(newInstructionIndex);

		// Check if operands are compatible with the new instruction.
		// If not: mutate them
		const Instructions::Instruction& instruction = line.getEnvironment().getInstructionSet().getInstruction(newInstructionIndex);
		for (uint64_t i = 0; i < instruction.getNbOperands(); i++) {
			const std::type_info& type = instruction.getOperandTypes().at(i).get();
			uint64_t dataSourceIndex = line.getOperand(i).first;
			bool isValid = false;
			if (dataSourceIndex == 0) {
				// regsister
				isValid = (type == typeid(PrimitiveType<double>));
			}
			else {
				// not register
				const DataHandlers::DataHandler& dataSource = line.getEnvironment().getDataSources().at(dataSourceIndex - 1).get();
				isValid = dataSource.canHandle(type);
			}
			// Alter the operand if needed
			if (!isValid) {
				// Force only the change of data source (location can remain unchanged thanks to scaling).
				bool forcedChangeWorked = initRandomCorrectLineOperand(instruction, line, i, true, false, true);
				if (!forcedChangeWorked) {
					// in case the forced change didn't work, no data source has the right type
					// for the given instruction. fail.
					// This should never happen if there is a check for Instructions viability in the Environment
					// Construction.
					throw std::runtime_error("An Instruction has no valid dataSource in the DataHandlers.");
				}
			}
		}
	}
	else if (selectedBit < lineSize.nbInstructionBits + lineSize.nbDestinationBits) {
		// DestinationIndex
		// Select a random destination (different from the current one)
		const uint64_t currentDestinationIndex = line.getDestinationIndex();
		uint64_t newDestinationIndex = RNG::getUnsignedInt64(0, line.getEnvironment().getNbRegisters() - 2);
		newDestinationIndex += (newDestinationIndex >= currentDestinationIndex) ? 1 : 0;
		line.setDestinationIndex(newDestinationIndex);
	}
	else if (selectedBit < lineSize.nbInstructionBits + lineSize.nbDestinationBits + lineSize.nbOperandsBits) {
		// Which operand is selected
		// Equal position of selectedBit within operand bits, divided by the total number of bits per operand.
		const uint64_t operandIndex = (selectedBit - (lineSize.nbInstructionBits + lineSize.nbDestinationBits)) / (lineSize.nbOperandDataSourceIndexBits + lineSize.nbOperandLocationBits);
		const uint64_t currentOperandDataSourceIndex = line.getOperand(operandIndex).first;
		const uint64_t currentOperandLocation = line.getOperand(operandIndex).second;
		const Instructions::Instruction& instruction = line.getEnvironment().getInstructionSet().getInstruction(line.getInstructionIndex());

		// Operands dataSourceIndex or Location
		// Same as before, but with modulo instead of division.
		// Result of modulo is compared with the number of bits per operand for the operandSourceIndex encoding 
		if (((selectedBit - (lineSize.nbInstructionBits + lineSize.nbDestinationBits)) % (lineSize.nbOperandDataSourceIndexBits + lineSize.nbOperandLocationBits)) < lineSize.nbOperandDataSourceIndexBits) {
			// Operand data source index
			initRandomCorrectLineOperand(instruction, line, operandIndex, true, false, true);
		}
		else {
			// Location (no fail thanks to scaling)
			initRandomCorrectLineOperand(instruction, line, operandIndex, false, true, true);
		}
	}
	else {
		// Parameters
		// Which Parameter is selected
		const uint64_t parameterIndex = (selectedBit - (lineSize.totalNbBits - lineSize.nbParametersBits)) / (lineSize.nbParametersBits / line.getEnvironment().getMaxNbParameters());
		// should not cause any division by zero since if there is no parameter in an environment, this code should never be reached.

		// Select a random parameterValue 
		// (do not bother to make it different the probability is too low and 
		// it will be detected through neutrality tests)
		Parameter newParameter = (int16_t)(Mutator::RNG::getUnsignedInt64(0, ((int64_t)PARAM_INT_MAX - (int64_t)PARAM_INT_MIN)) + (int64_t)PARAM_INT_MIN);
		line.setParameter(parameterIndex, newParameter);
	}
}
