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
* \return true if the operand was successfully initialized, false if no valid
* data source could be found for this Instruction and operandIdx couple.
*/
static bool initRandomCorrectLineOperand(const Instructions::Instruction& instruction, Program::Line& line, const uint64_t& operandIdx)
{
	const Environment& env = line.getEnvironment();

	// Select an operand 
	// The type of operand needed
	const type_info& operandType = instruction.getOperandTypes().at(operandIdx).get();

	// keep a record of tested indexes
	std::set<uint64_t> operandDataSourceIndexes;

	bool operandFound = false;
	while (!operandFound && operandDataSourceIndexes.size() < env.getNbDataSources()) {
		// Select an operandDataSourceIndex
		uint64_t operandDataSourceIndex = Mutator::RNG::getUnsignedInt64(0, (env.getNbDataSources() - 1) - operandDataSourceIndexes.size());
		// Correct the index with the number of already tested ones inferior to it.
		// This works because the set is ordered
		std::for_each(operandDataSourceIndexes.begin(), operandDataSourceIndexes.end(),
			[&operandDataSourceIndex](uint64_t index) { if (index <= operandDataSourceIndex) operandDataSourceIndex++; });
		// Add the index to the set
		operandDataSourceIndexes.insert(operandDataSourceIndex);

		// check if the selected dataSource can provide the type requested by the instruction
		// and if so get the addressable space
		size_t addressableSpace = 0;
		if (operandDataSourceIndex == 0) {
			// Data Source is the registers
			if (operandType == typeid(PrimitiveType<double>)) {
				operandFound = true;
				addressableSpace = env.getNbRegisters();
			}
		}
		else {
			// Data source is a dataHandler
			if (env.getDataSources().at(operandDataSourceIndex - 1).get().canHandle(operandType)) {
				operandFound = true;
				addressableSpace = env.getDataSources().at(operandDataSourceIndex - 1).get().getAddressSpace(operandType);
			}
		}

		// The data source can provide the required data type
		if (operandFound) {
			// Select a location
			uint64_t operandLocation = Mutator::RNG::getUnsignedInt64(0, addressableSpace - 1);
			// set line operand info
			line.setOperand(operandIdx, operandDataSourceIndex, operandLocation);
		}
	}

	return operandFound;
}

void Mutator::Line::initRandomCorrectLine(Program::Line& line)
{
	const Environment& env = line.getEnvironment();

	// Select and set a destinationIndex. (can not fail)
	uint64_t destinationIndex = RNG::getUnsignedInt64(0, env.getNbRegisters() - 1);
	line.setDestinationIndex(destinationIndex); // Should never throw.. but I did not deactivate the check anyway.

	// Select and set all parameter values. (can not fail)
	for (uint64_t paramIdx = 0; paramIdx < env.getMaxNbParameters(); paramIdx++) {
		Parameter param((int32_t)(Mutator::RNG::getUnsignedInt64(0, ((int64_t)PARAM_INT_MAX - (int64_t)PARAM_INT_MIN)) + (int64_t)PARAM_INT_MIN));
		line.setParameter(paramIdx, param);
	}

	// Detecting impossible combination may imply reversing some random choice
	// and making sure not to re-select the same option again. The following 
	// vectors are used to check that.
	std::set<uint64_t> instructionIndexes;

	// Boolean value set to true only when all attributes of the line have been successfully initialized.
	bool allGood = false;
	while (!allGood) {
		// Check if all instructions have already been tested.
		if (instructionIndexes.size() == env.getNbInstructions()) {
			throw std::runtime_error("No instruction of the environment can be called with valid data.");
		}

		// Select an instruction within remaining ones.
		uint64_t instructionIndex = RNG::getUnsignedInt64(0, (env.getNbInstructions() - 1) - instructionIndexes.size());
		// Correct the index with the number of already tested ones inferior to it.
		std::for_each(instructionIndexes.begin(), instructionIndexes.end(),
			[&instructionIndex](uint64_t index) { if (index <= instructionIndex) instructionIndex++; });
		// Get the instruction
		const Instructions::Instruction& instruction = env.getInstructionSet().getInstruction(instructionIndex);
		// Set the instructionIndex
		line.setInstructionIndex(instructionIndex); // Should never throw.. but I did not deactivate the check anyway.
		// Add the index to the set
		instructionIndexes.insert(instructionIndex);

		// Select operands needed by the instruction
		uint64_t operandIdx = 0;
		for (; operandIdx < instruction.getNbOperands(); operandIdx++) {

			// Check if all operands were tested (and none were valid)
			bool operandFound = initRandomCorrectLineOperand(instruction, line, operandIdx);

			if (!operandFound) {
				// If the algorithm failed to find a dataSource providing the right type of data
				// stop the search for more operand and try a new instruction.
				break; // for(operandIdx)-loop
			}
		}

		// If not all operands of the instruction were successfully chosen
		if (operandIdx < instruction.getNbOperands()) {
			continue; // Next iteration of the while(allGood) loop to search for a new instruction.
		}

		// Select operands not used by the instruction (can not fail)
		for (; operandIdx < env.getMaxNbOperands(); operandIdx++) {
			// Select a random dataSource
			uint64_t operandDataSourceIndex = Mutator::RNG::getUnsignedInt64(0, env.getNbDataSources() - 1);
			// Select a location
			uint64_t operandLocation;
			if (operandDataSourceIndex == 0) {
				// Data source is a register
				operandLocation = Mutator::RNG::getUnsignedInt64(0, env.getNbRegisters() - 1);
			}
			else {
				// Data source is not a register
				operandLocation = Mutator::RNG::getUnsignedInt64(0, env.getDataSources().at(operandDataSourceIndex - 1).get().getLargestAddressSpace() - 1);
			}
			line.setOperand(operandIdx, operandDataSourceIndex, operandLocation);
		}

		// If all went well during operand selection.
		allGood = true;
	}
}
