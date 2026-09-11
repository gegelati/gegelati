
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


bool Evolution::Representation::isValid(const Genotype& genotype) const 
{
    std::unique_ptr<Node::GenotypeTemplate> genTemplate = std::move(this->getGenotypeTemplate());

    if(genotype.getSize() != genTemplate->size()) {
        // Not the same number of node groups
        return false;
    }


    // Check validity of each group
    for(size_t idxGroup = 0; idxGroup < genotype.getSize(); idxGroup++) {
        const Node::NodeGroup& group = genotype.getNodeGroup(idxGroup);

        // Number of nodes in the group is wrong
        if(group.getSize() < genTemplate->getRangeAt(idxGroup).first || group.getSize() > genTemplate->getRangeAt(idxGroup).second){
            return false;
        }

        const Node::NodeTemplate& nodeTemplate = genTemplate->getNodeTemplateAt(idxGroup);

        // Check validity of each node
        for(size_t idxNode = 0; idxNode < group.getSize(); idxNode++) {
            const Node::GPNode& node = group.getNode(idxNode);

            if(node.getSize() != nodeTemplate.size()) {
                // One node has an unexpected number of values
                return false;
            }

            // Check validity of each value.
            for(size_t idxValue = 0; idxValue < node.getSize(); idxValue++) {
                const Data::DataValue& value = node.getValue(idxValue);
                const Dimensions::Constraint& constraint = nodeTemplate.getConstraintAt(idxValue);
                if(!constraint.accepts(value)) {
                    // Value is not accepted
                    return false;
                }
            }
        }
    }

    // Individual has a valid genotype
    return true;
    
}

Dimensions::DimensionFlow Evolution::Representation::getDimensionFlow() const
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
    Data::DataValue resultIndiv = this->executeGenotype(genotype, inputSources);
    for(const auto& function: this->outputFunctions) {
        resultIndiv = function->execute(resultIndiv);
    }
    return resultIndiv;
}