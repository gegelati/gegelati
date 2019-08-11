#include "environment.h"

size_t Environment::computeLargestAddressSpace(const std::vector<std::reference_wrapper<DataHandlers::DataHandler>>& dHandlers)
{
	size_t res{ 0 };
	for (auto dHandler : dHandlers) {
		size_t addressSpace = dHandler.get().getLargestAddressSpace();
		res = (addressSpace > res) ? addressSpace : res;
	}
	return res;
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

const std::vector<std::reference_wrapper<DataHandlers::DataHandler>>& Environment::getDataSources() const
{
	return this->dataSources;
}

const Instructions::Set& Environment::getInstructionSet() const
{
	return this->instructionSet;
}
