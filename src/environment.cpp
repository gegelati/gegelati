#include <stdexcept>

#include "environment.h"

size_t Environment::computeLargestAddressSpace(const size_t nbRegisters, const std::vector<std::reference_wrapper<DataHandlers::DataHandler>>& dHandlers)
{
	size_t res{ nbRegisters };
	for (auto dHandler : dHandlers) {
		size_t addressSpace = dHandler.get().getLargestAddressSpace();
		res = (addressSpace > res) ? addressSpace : res;
	}
	return res;
}

const size_t Environment::computeLineSize(const Environment& env)
{
	// $ceil(log2(n)) + ceil(log2(i)) + m * (ceil(log2(nb_{ src })) + ceil(log2(largestAddressSpace)) + p * sizeof(Param)_{inByte} * 8$
	const size_t n = env.getNbRegisters();

	const size_t i = env.getNbInstructions();

	const size_t m = env.getMaxNbOperands();

	const size_t nbSrc = env.getNbDataSources();

	const size_t largestAddressSpace = env.getLargestAddressSpace();

	const size_t p = env.getMaxNbParameters();

	// Add some checks on values. Only p can be null for a valid program. nbSrc 
	// cannot be 1, as it would mean an environment with only registers.
	if (n == 0 || i == 0 || m == 0 || nbSrc <= 1 || largestAddressSpace == 0) {
		throw std::domain_error("Environment given to the computeLineSize is invalid for building a program." \
			"It is parameterized with no or only registers, contains no Instruction, Instruction" \
			" with no operands, no DataHandler or DataHandler with no addressable Space.");
	}

	return (size_t)(ceill(log2l((long double)n)) + ceill(log2l((long double)i))
		+ m * (ceill(log2l((long double)nbSrc) + ceill(log2l((long double)largestAddressSpace))))
		+ p * sizeof(Parameter) * 8);
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

size_t Environment::getLineSize() const
{
	return this->lineSize;
}

const std::vector<std::reference_wrapper<DataHandlers::DataHandler>>& Environment::getDataSources() const
{
	return this->dataSources;
}

const Instructions::Set& Environment::getInstructionSet() const
{
	return this->instructionSet;
}
