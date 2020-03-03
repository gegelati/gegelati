#include <stdexcept>

#include "environment.h"

size_t Environment::computeLargestAddressSpace(const size_t nbRegisters, const std::vector<std::reference_wrapper<const Data::DataHandler>>& dHandlers)
{
	size_t res{ nbRegisters };
	for (auto dHandler : dHandlers) {
		size_t addressSpace = dHandler.get().getLargestAddressSpace();
		res = (addressSpace > res) ? addressSpace : res;
	}
	return res;
}

Instructions::Set Environment::filterInstructionSet(const Instructions::Set& iSet, const std::vector < std::reference_wrapper<const Data::DataHandler>>& dataSources) {
	Instructions::Set filteredSet;

	// Check if all instructions can be used for the given DataHandlers
	for (uint64_t idxInstruction = 0; idxInstruction < iSet.getNbInstructions(); idxInstruction++) {
		const Instructions::Instruction& instruction = iSet.getInstruction(idxInstruction);
		// Check that all operands can be provided by at least one DataHandler
		bool allOperandsHandled = true;
		for (uint64_t idxOperand = 0; idxOperand < instruction.getNbOperands(); idxOperand++) {
			const std::type_info& type = instruction.getOperandTypes().at(idxOperand).get();
			// Check DataHandlers for this type until one is found
			bool isHandled = false;
			// Check registers double first
			if (type == typeid(double)) {
				// The type is handled by one dataHandler, stop searching for more.
				isHandled = true;
				break;
			}

			for (const auto& dHandler : dataSources) {
				if (dHandler.get().canHandle(type)) {
					// The type is handled by one dataHandler, stop searching for more.
					isHandled = true;
					break;
				}
			}

			// If the dataType is not handled, throw an exception.
			if (!isHandled) {
				std::cout << "An instruction with an operand of type " << type.name()
					<< " is ignored when building the Environment because"
					<< " no dataSource can provide data for this operand type.";

				// break of the operand loop for this instruction.
				allOperandsHandled = false;
				break;
			}
		}

		// If all operands are handled, add it to the filtered Instruction set.
		if (allOperandsHandled) {
			filteredSet.add(iSet.getInstruction(idxInstruction));
		}
	}

	return filteredSet;
}

const LineSize Environment::computeLineSize(const Environment& env)
{
	// $ ceil(log2(i))+ ceil(log2(n)) + m * (ceil(log2(nb_{ src })) + ceil(log2(largestAddressSpace)) + p * sizeof(Param)_{inByte} * 8$
	const size_t n = env.getNbRegisters();

	const size_t i = env.getNbInstructions();

	const size_t m = env.getMaxNbOperands();

	const size_t nbSrc = env.getNbDataSources();

	const size_t largestAddressSpace = env.getLargestAddressSpace();

	const size_t p = env.getMaxNbParameters();

	// Add some checks on values. Only p can be null for a valid program. nbSrc 
	// cannot be 1, as it would mean an environment with only registers.
	// i cannot be 1 also because this would mean a unique instruction 
	// (although feasible.. I prefer to forbid it for now to avoid complicating
	// the line mutators). 
	if (n == 0 || i <= 1 || m == 0 || nbSrc <= 1 || largestAddressSpace == 0) {
		throw std::domain_error("Environment given to the computeLineSize is invalid for building a program." \
			"It is parameterized with no or only registers, contains no Instruction, Instruction" \
			" with no operands, no DataHandler or DataHandler with no addressable Space.");
	}
	LineSize result;
	result.nbInstructionBits = (size_t)(ceill(log2l((long double)n)));
	result.nbDestinationBits = (size_t)ceill(log2l((long double)i));
	result.nbOperandDataSourceIndexBits = (size_t)(ceill(log2l((long double)nbSrc)));
	result.nbOperandLocationBits = (size_t)ceill(log2l((long double)largestAddressSpace));
	result.nbOperandsBits = (size_t)(m * (result.nbOperandDataSourceIndexBits + result.nbOperandLocationBits));
	result.nbParametersBits = (size_t)(p * sizeof(Parameter) * 8);

	result.totalNbBits = result.nbInstructionBits + result.nbDestinationBits + result.nbOperandsBits + result.nbParametersBits;

	return result;

}

size_t Environment::getNbRegisters() const
{
	return this->nbRegisters;
}

size_t Environment::getNbInstructions() const
{
	return this->nbInstructions;
}

size_t Environment::getMaxNbOperands() const
{
	return this->maxNbOperands;
}

size_t Environment::getMaxNbParameters() const
{
	return this->maxNbParameters;
}

size_t Environment::getNbDataSources() const
{
	return this->nbDataSources;
}

size_t Environment::getLargestAddressSpace() const
{
	return this->largestAddressSpace;
}

const LineSize& Environment::getLineSize() const
{
	return this->lineSize;
}

const std::vector<std::reference_wrapper<const Data::DataHandler>>& Environment::getDataSources() const
{
	return this->dataSources;
}

const Instructions::Set& Environment::getInstructionSet() const
{
	return this->instructionSet;
}
