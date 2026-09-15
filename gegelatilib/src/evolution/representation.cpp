
#include "evolution/representation.h"

#include "evolution/individual.h"

std::unique_ptr<Evolution::Representation> Evolution::Representation::cloneUniquePtr() const
{
    std::unique_ptr<Evolution::Representation> clone = this->cloneOnlyRepresentation();
    for(const std::unique_ptr<Dimensions::ActivationFunctions::Function>& function: this->outputFunctions) {
        clone->addOutputFunction(function->cloneUniquePtr());
    }
    return std::move(clone);
}

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

const Node::GenotypeConstraint& Evolution::Representation::getGenotypeConstraint() const
{
    return *this->genotypeConstraint;
}

bool Evolution::Representation::isValid(const Genotype& genotype) const 
{
    if(this->genotypeConstraint == nullptr || genotype.getSize() != this->genotypeConstraint->size()) {
        // Not the same number of node groups
        return false;
    }


    // Check validity of each group
    for(size_t idxGroup = 0; idxGroup < genotype.getSize(); idxGroup++) {
        const Node::NodeGroup& group = genotype.getNodeGroup(idxGroup);

        // Number of nodes in the group is wrong
        if(group.getSize() < this->genotypeConstraint->getRangeAt(idxGroup).first || group.getSize() > this->genotypeConstraint->getRangeAt(idxGroup).second){
            return false;
        }

        const Node::NodeConstraint& nodeConstraint = this->genotypeConstraint->getNodeConstraintAt(idxGroup);

        // Check validity of each node
        for(size_t idxNode = 0; idxNode < group.getSize(); idxNode++) {
            const Node::GPNode& node = group.getNode(idxNode);

            if(node.getSize() != nodeConstraint.size()) {
                // One node has an unexpected number of values
                return false;
            }

            // Check validity of each value.
            for(size_t idxValue = 0; idxValue < node.getSize(); idxValue++) {
                const Data::DataValue& value = node.getValue(idxValue);
                const Dimensions::Constraint& constraint = nodeConstraint.getConstraintAt(idxValue);

                // Value is not directly accepted, check if it is an individual, and if yes, if it is accepted.
                // For now it is forced shared_ptr of const individual... 
                if(value.getElementType() == typeid(std::shared_ptr<const Individual>)) {
                    // Get individual
                    const std::shared_ptr<const Evolution::Individual>& individualValue = value.getScalar<std::shared_ptr<const Individual>>();
                    if(!individualValue->isValid()) {
                        return false;
                    }

                    // Individual must support the current representation inputs
                    const std::vector<Dimensions::Requirement>& inputDims = individualValue->getRepresentation().getDimensionFlow().getInputDimensions();
                    if(!Dimensions::DimensionFlow::acceptsRequirements(this->dimensionFlow.getInputDimensions(), inputDims)) {
                        return false;
                    }

                    // Individual must output a scalar with the required constraint.
                    const Dimensions::Requirement & outputDim = individualValue->getRepresentation().getDimensionFlow().getOutputDimension();
                    if(!outputDim.getConstraint().isCompatibleWith(constraint)) {
                        return false;
                    }
                } else if(!constraint.accepts(value)) {
                    return false;
                }
            }
        }
    }

    // Genotype is valid
    return true;
    
}

const Dimensions::DimensionFlow& Evolution::Representation::getDimensionFlow() const
{
    return this->dimensionFlow;
}

std::string Evolution::Representation::summary() const
{
    return this->dimensionFlow.summary();
}

Data::DataValue Evolution::Representation::execute(
          const Genotype& genotype, const std::vector<Data::DataView>& inputSources) const
{
    const std::vector<Dimensions::Requirement>& inputDim = this->dimensionFlow.getInputDimensions();
    if(inputSources.size() != inputDim.size()) {
        throw std::runtime_error("Evolution::Representation::execute: Dimensions of the input sources are wrong");
    }
    for(size_t idx = 0; idx < inputSources.size(); idx++){
        if(!inputDim.at(0).accepts(inputSources.at(0))) {
            throw std::runtime_error("Evolution::Representation::execute: Dimensions of the input sources are wrong");
        }
    }

    Data::DataValue resultIndiv = this->executeGenotype(genotype, inputSources);
    for(const auto& function: this->outputFunctions) {
        resultIndiv = function->execute(resultIndiv);
    }
    return resultIndiv;
}