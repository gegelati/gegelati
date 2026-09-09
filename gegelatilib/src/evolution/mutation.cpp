#include "evolution/mutation.h"

#include "evolution/individual.h"



Data::DataValue Evolution::Mutation::sampleNodeValue(const Node::NodeValueRequirement& nodeValueRequirements, RNG::RNG& rng)
{
    /*
    if(nodeValueRequirements.size() == 0) {
        throw std::runtime_error("Evolution::Mutation::sampleNodeValue: NodeRequirements is empty :(.");
    }
    // Default select first requirements
    size_t idxRequirements = 0;

    // Sample a random requirements if more than one proposed
    if(nodeValueRequirements.size() > 1) {
        idxRequirements = rng.getUnsignedInt64(0, nodeValueRequirements.size() - 1);
    }

    const Node::NodeValueConfiguration& valueRequirements = *nodeValueRequirements.getconfigurationAt(idxRequirements);
    Node::NodeValue value;

    if(std::holds_alternative<Node::NodeValueRange>(valueRequirements)) {
        // Value should be selected in a specified range
        const Node::NodeValueRange& nodeValueRange = std::get<Node::NodeValueRange>(valueRequirements);

        if(std::holds_alternative<std::pair<size_t, size_t>>(nodeValueRange)) {
            // Range of unsigned int
            const std::pair<size_t, size_t>& uintPair = std::get<std::pair<size_t, size_t>>(nodeValueRange);
            value = rng.getUnsignedInt64(uintPair.first, uintPair.second - 1);

        } else { //if (std::holds_alternative<std::pair<double, double>>(nodeValueRange)) { commented because in the idea its important, but impossible to cover with current configuration
            // Range of double
            const std::pair<double, double>& doublePair = std::get<std::pair<double, double>>(nodeValueRange);
            value = rng.getDouble(doublePair.first, doublePair.second);
        }

    } else  if (std::holds_alternative<std::vector<Node::NodeValue>>(valueRequirements)) { 

        // List of node values.
        const std::vector<Node::NodeValue>& nodeValues = std::get<std::vector<Node::NodeValue>>(valueRequirements);

        // Sample a random value.
        value = nodeValues.at(rng.getUnsignedInt64(0, nodeValues.size() - 1));

    } else { //std::holds_alternative<std::vector<std::weak_ptr<const Evolution::Individual>>>(valueRequirements)

        // List of weark ptr of const individuals.
        const std::vector<std::weak_ptr<const Evolution::Individual>>& nodeValues = std::get<std::vector<std::weak_ptr<const Evolution::Individual>>>(valueRequirements);

        // Sample a random individual and get its shared_ptr
        value = nodeValues.at(rng.getUnsignedInt64(0, nodeValues.size() - 1)).lock();

    }

    return value;*/
    return Data::DataValue::scalar<size_t>(0);
}


std::unique_ptr<Node::GPNode> Evolution::Mutation::createRandomNode(const Node::NodeRequirements& nodeRequirements, RNG::RNG& rng)
{
    if(nodeRequirements.size() == 0) {
        throw std::runtime_error("Evolution::Mutation::createRandomNode: NodeRequirements is empty.");
    }

    std::vector<Data::DataValue> values;
    for(size_t idxValue = 0; idxValue < nodeRequirements.size(); idxValue++) {
        values.push_back(this->sampleNodeValue(nodeRequirements.getValueRequirementsAt(idxValue), rng));
    }
    return std::make_unique<Node::GPNode>(values);
}



void Evolution::Mutation::initRandomGenotype(Genotype& genotype, const Node::GenotypeRequirements& genotypeRequirements, RNG::RNG& rng)
{
    if(genotypeRequirements.size() == 0) {
        throw std::runtime_error("Evolution::Mutation::initRandomGenotype: genotypeRequirements is empty.");
    }

    for(size_t idxRequirements = 0; idxRequirements < genotypeRequirements.size(); idxRequirements++) {
        Node::NodeGroup& nodeGroup = genotype.addNodeGroup();
        
        const std::pair<size_t, size_t>& range = genotypeRequirements.getRangeAt(idxRequirements);
        size_t nbNodesOfRequirements = rng.getUnsignedInt64(range.first, range.second);

        for(size_t idxNode = 0; idxNode < nbNodesOfRequirements; idxNode++) {
            nodeGroup.addNode(this->createRandomNode(genotypeRequirements.getNodeRequirementsAt(idxRequirements), rng));
        }
    }
}

void Evolution::Mutation::mutateNode(Node::GPNode& node, const Node::NodeRequirements& nodeRequirements, RNG::RNG& rng)
{
    if(nodeRequirements.size() != node.getSize()) {
        throw std::runtime_error("Evolution::Mutation::mutateNode: NodeRequirements size does not correspond to the genotypeidual.");
    }
    size_t idxValueMutated;
    /*Data::DataValue newValue;
    // Simple loop to ensure one value is mutated.
    do {
        idxValueMutated = rng.getUnsignedInt64(0, node.getSize() - 1);
        newValue = this->sampleNodeValue(*nodeRequirements.getValueRequirementsAt(idxValueMutated), rng);
    } while (node.getValue(idxValueMutated) == newValue);


    node.setValue(idxValueMutated, newValue);
    
    */
}


void Evolution::Mutation::mutateGenotype(Genotype& genotype, const Node::GenotypeRequirements& genotypeRequirements, RNG::RNG& rng)
{
    if(genotypeRequirements.size() == 0) {
        throw std::runtime_error("Evolution::Mutation::mutateGenotype: genotypeRequirements is empty.");
    }

    double pMutateNode = 0.5;

    const Node::NodeRequirements& nodeRequirements = genotypeRequirements.getNodeRequirementsAt(0);
    for(size_t idxNodeGroup = 0; idxNodeGroup < genotype.getSize(); idxNodeGroup++) {
        Node::NodeGroup& nodeGroup = genotype.getMutableNodeGroup(idxNodeGroup);

        for(size_t idxNode = 0; idxNode < nodeGroup.getSize(); idxNode++) {
            if(rng.getDouble(0, 1) < pMutateNode) {
                this->mutateNode(nodeGroup.getMutableNode(idxNode), nodeRequirements, rng);
            }
        }
    }
}