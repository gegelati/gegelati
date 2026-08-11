
#include "evolution/representation.h"

size_t Evolution::Representation::getMinNbNodes() const
{
    return this->nbNodesMin;
}

size_t Evolution::Representation::getMaxNbNodes() const
{
    return this->nbNodesMax;
}

void Evolution::Representation::setInputDimensions(const std::vector<std::reference_wrapper<const Data::DataHandler>>& inputSources)
{
    this->nbInputSources = inputSources.size();
    this->maxInputSourceIdx = 0;
    for (const Data::DataHandler& dHandler : inputSources) {
        size_t addressSpace = dHandler.getLargestAddressSpace();
        this->maxInputSourceIdx = (addressSpace > this->maxInputSourceIdx) ? addressSpace : this->maxInputSourceIdx;
    }
}

size_t Evolution::Representation::getNbInputSources() const
{
    return this->nbInputSources;
}

size_t Evolution::Representation::getMaxInputSourceIdx() const
{
    return this->maxInputSourceIdx;
}

