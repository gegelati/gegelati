
#include "representations/representation.h"

#include "individual.h"

std::unique_ptr<Representations::Representation> Representations::Representation::cloneUniquePtr() const
{
    std::unique_ptr<Representations::Representation> clone = this->cloneOnlyRepresentation();
    for(const std::unique_ptr<Dimensions::ActivationFunctions::Function>& function: this->outputFunctions) {
        clone->addOutputFunction(function->cloneUniquePtr());
    }
    return std::move(clone);
}

void Representations::Representation::addOutputFunction(std::unique_ptr<Dimensions::ActivationFunctions::Function> function)
{
    this->dimensionFlow.addLayer(function->name(), function->inputDimensions(), function->outputDimension());
    if(!this->dimensionFlow.isValid()) {
        throw std::runtime_error("Representations::Representation::addOutputFunction: Function " + function->name() +" cannot be add to current flow: " + this->dimensionFlow.summary());
    }
    this->outputFunctions.push_back(std::move(function));
    
}

const GraphBased::GenotypeConstraint& Representations::Representation::getGenotypeConstraint() const
{
    return *this->genotypeConstraint;
}

bool Representations::Representation::isValid(const GraphBased::Genotype& genotype) const 
{
    if(this->genotypeConstraint == nullptr || genotype.getSize() != this->genotypeConstraint->size()) {
        // Not the same number of node groups
        return false;
    }


    // Check validity of each group
    for(size_t idxGroup = 0; idxGroup < genotype.getSize(); idxGroup++) {
        const GraphBased::NodeGroup& group = genotype.getNodeGroup(idxGroup);

        // Number of nodes in the group is wrong
        if(group.getSize() < this->genotypeConstraint->getRangeAt(idxGroup).first || group.getSize() > this->genotypeConstraint->getRangeAt(idxGroup).second){
            return false;
        }

        const GraphBased::NodeConstraint& nodeConstraint = this->genotypeConstraint->getNodeConstraintAt(idxGroup);

        // Check validity of each node
        for(size_t idxNode = 0; idxNode < group.getSize(); idxNode++) {
            const GraphBased::GPNode& node = group.getNode(idxNode);

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
                    const std::shared_ptr<const Individual>& individualValue = value.getScalar<std::shared_ptr<const Individual>>();
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

const Dimensions::DimensionFlow& Representations::Representation::getDimensionFlow() const
{
    return this->dimensionFlow;
}

std::string Representations::Representation::summary() const
{
    return this->dimensionFlow.summary();
}

Data::DataValue Representations::Representation::execute(
          const GraphBased::Genotype& genotype, const std::vector<Data::DataView>& inputSources) const
{
    // No need to check if the dimension flow is valid, it is necessarily valid by construction for now.

    // Check inputs are valid
    const std::vector<Dimensions::Requirement>& inputDim = this->dimensionFlow.getInputDimensions();
    if(inputSources.size() != inputDim.size()) {
        throw std::runtime_error("Representations::Representation::execute: Dimensions of the input sources are wrong");
    }
    for(size_t idx = 0; idx < inputSources.size(); idx++){
        if(!inputDim.at(0).accepts(inputSources.at(0))) {
            throw std::runtime_error("Representations::Representation::execute: Dimensions of the input sources are wrong");
        }
    }

    Data::DataValue resultIndiv = this->executeGenotype(genotype, inputSources);
    for(const auto& function: this->outputFunctions) {
        resultIndiv = function->execute(resultIndiv);
    }
    return resultIndiv;
}