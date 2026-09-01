
#include "evolution/representation.h"

size_t Evolution::Representation::getMinNbNodes() const
{
    return this->nbNodesMin;
}

size_t Evolution::Representation::getMaxNbNodes() const
{
    return this->nbNodesMax;
}

void Evolution::Representation::setInputDimensions(const std::vector<Data::DataView>& inputSources)
{
    this->nbInputSources = inputSources.size();
    this->maxInputSourceIdx = 0;
    for (const Data::DataView& dHandler : inputSources) {
        size_t addressSpace = dHandler.getAddressSpace();
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

void Evolution::Representation::setTangled(bool tangled)
{
    this->tangled = tangled;
}

bool Evolution::Representation::isTangled() const
{
    return this->tangled;
}

void Evolution::Representation::setTangledPopulation(const Population& tangledPop)
{
    if(!this->tangled) {
        throw std::runtime_error("Evolution::Representation::setTangledPopulation: cannot set a tangled population with representation is not tangled!");
    }

    this->tangledPopulation = tangledPop;
}

bool Evolution::Representation::hasTangledPopulation()
{
    return this->tangledPopulation.has_value();
}

const std::optional<std::reference_wrapper<const Evolution::Population>>& Evolution::Representation::getTangledPopulation()
{
    return this->tangledPopulation;
}