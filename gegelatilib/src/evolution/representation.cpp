
#include "evolution/representation.h"

size_t Evolution::Representation::getMinNbNodes() const
{
    return this->nbNodesMin;
}

size_t Evolution::Representation::getMaxNbNodes() const
{
    return this->nbNodesMax;
}

void Evolution::Representation::setDimensions(const std::vector<Data::DataRequirement>& inputDimensions, const Data::DataRequirement& outputDimension)
{
    this->inputDimensions = inputDimensions;
    this->outputDimension = outputDimension;
}

const std::vector<Data::DataRequirement>& Evolution::Representation::getInputDimensions() const
{
    return this->inputDimensions;
}

const Data::DataRequirement& Evolution::Representation::getOutputDimension() const
{
    return this->outputDimension;
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