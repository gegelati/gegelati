
#include "evolution/representation.h"

size_t Evolution::Representation::getMinNbNodes() const
{
    return this->nbNodesMin;
}

size_t Evolution::Representation::getMaxNbNodes() const
{
    return this->nbNodesMax;
}

void Evolution::Representation::addOutputFunction(std::unique_ptr<Dimensions::ActivationFunctions::Function> function)
{
    this->dimensionFlow.addLayer(function->name(), function->inputDimensions(), function->outputDimension());
    if(!this->dimensionFlow.isValid()) {
        throw std::runtime_error("Evolution::Representation::addOutputFunction: Function " + function->name() +" cannot be add to current flow: " + this->dimensionFlow.summary());
    }
    this->outputFunctions.push_back(std::move(function));
    
}

Dimensions::ControlFlow Evolution::Representation::getControlFlow() const
{
    return this->dimensionFlow;
}

std::string Evolution::Representation::summary() const
{
    return this->dimensionFlow.summary();
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

Data::DataValue Evolution::Representation::executeIndividual(
          const Individual& indiv, const std::vector<Data::DataView>& inputSources) const
{
    Data::DataValue resultIndiv = this->executeIndividualRaw(indiv, inputSources);
    for(const auto& function: this->outputFunctions) {
        resultIndiv = function->execute(resultIndiv);
    }
    return resultIndiv;
}