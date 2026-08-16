#include "evolution/mutation.h"

#include "evolution/individual.h"



Node::NodeValue Evolution::Mutation::sampleNodeValue(const Node::NodeValueTemplate& nodeValueTemplate, RNG::RNG& rng)
{
    if(nodeValueTemplate.size() == 0) {
        throw std::runtime_error("Evolution::Mutation::sampleNodeValue: NodeTemplates is empty :(.");
    }
    // Default select first template
    size_t idxTemplate = 0;

    // Sample a random template if more than one proposed
    if(nodeValueTemplate.size() > 1) {
        idxTemplate = rng.getUnsignedInt64(0, nodeValueTemplate.size() - 1);
    }

    const Node::NodeValueConfiguration& valueTemplate = *nodeValueTemplate.getconfigurationAt(idxTemplate);
    Node::NodeValue value;

    if(std::holds_alternative<Node::NodeValueRange>(valueTemplate)) {
        // Value should be selected in a specified range
        const Node::NodeValueRange& nodeValueRange = std::get<Node::NodeValueRange>(valueTemplate);

        if(std::holds_alternative<std::pair<size_t, size_t>>(nodeValueRange)) {
            // Range of unsigned int
            const std::pair<size_t, size_t>& uintPair = std::get<std::pair<size_t, size_t>>(nodeValueRange);
            value = rng.getUnsignedInt64(uintPair.first, uintPair.second - 1);

        } else { //if (std::holds_alternative<std::pair<double, double>>(nodeValueRange)) { commented because in the idea its important, but impossible to cover with current configuration
            // Range of double
            const std::pair<double, double>& doublePair = std::get<std::pair<double, double>>(nodeValueRange);
            value = rng.getDouble(doublePair.first, doublePair.second);
        }

    } else { // if (std::holds_alternative<std::vector<Node::NodeValue>>(valueTemplate)) { commented because in the idea its important, but impossible to cover with current configuration

        // List of node values.
        const std::vector<Node::NodeValue>& nodeValues = std::get<std::vector<Node::NodeValue>>(valueTemplate);

        // Sample a random value.
        value = nodeValues.at(rng.getUnsignedInt64(0, nodeValues.size() - 1));

    }

    return value;
}


std::unique_ptr<Node::GPNode> Evolution::Mutation::createRandomNode(const Node::NodeTemplate& nodeTemplate, RNG::RNG& rng)
{
    if(nodeTemplate.size() == 0) {
        throw std::runtime_error("Evolution::Mutation::createRandomNode: NodeTemplates is empty.");
    }

    std::vector<Node::NodeValue> values;
    for(size_t idxValue = 0; idxValue < nodeTemplate.size(); idxValue++) {
        values.push_back(this->sampleNodeValue(*nodeTemplate.getValueTemplateAt(idxValue), rng));
    }
    return std::make_unique<Node::GPNode>(values);
}



void Evolution::Mutation::initRandomGenotype(Genotype& genotype, const Node::GenotypeTemplate& genotypeTemplate, RNG::RNG& rng)
{
    if(genotypeTemplate.size() == 0) {
        throw std::runtime_error("Evolution::Mutation::initRandomGenotype: genotypeTemplate is empty.");
    }

    for(size_t idxTemplate = 0; idxTemplate < genotypeTemplate.size(); idxTemplate++) {
        genotype.addNodeGroup();
        Node::NodeGroup& nodeGroup = genotype.getMutableNodeGroup(genotype.getSize() - 1);
        
        const std::pair<size_t, size_t>& range = genotypeTemplate.getRangeAt(idxTemplate);
        size_t nbNodesOfTemplate = rng.getUnsignedInt64(range.first, range.second);

        for(size_t idxNode = 0; idxNode < nbNodesOfTemplate; idxNode++) {
            nodeGroup.addNode(this->createRandomNode(*genotypeTemplate.getNodeTemplateAt(idxTemplate), rng));
        }
    }
}

void Evolution::Mutation::mutateNode(Node::GPNode& node, const Node::NodeTemplate& nodeTemplate, RNG::RNG& rng)
{
    if(nodeTemplate.size() != node.getSize()) {
        throw std::runtime_error("Evolution::Mutation::mutateNode: NodeTemplates size does not correspond to the genotypeidual.");
    }
    size_t idxValueMutated;
    Node::NodeValue newValue;
    // Simple loop to ensure one value is mutated.
    do {
        idxValueMutated = rng.getUnsignedInt64(0, node.getSize() - 1);
        newValue = this->sampleNodeValue(*nodeTemplate.getValueTemplateAt(idxValueMutated), rng);
    } while (node.getValue(idxValueMutated) == newValue);

    node.setValue(idxValueMutated, newValue);
}


void Evolution::Mutation::mutateGenotype(Genotype& genotype, const Node::GenotypeTemplate& genotypeTemplate, RNG::RNG& rng)
{
    if(genotypeTemplate.size() == 0) {
        throw std::runtime_error("Evolution::Mutation::mutateGenotype: genotypeTemplate is empty.");
    }

    double pMutateNode = 0.5;

    const Node::NodeTemplate& nodeTemplate = *genotypeTemplate.getNodeTemplateAt(0);
    for(size_t idxNodeGroup = 0; idxNodeGroup < genotype.getSize(); idxNodeGroup++) {
        Node::NodeGroup& nodeGroup = genotype.getMutableNodeGroup(idxNodeGroup);

        for(size_t idxNode = 0; idxNode < nodeGroup.getSize(); idxNode++) {
            if(rng.getDouble(0, 1) < pMutateNode) {
                this->mutateNode(nodeGroup.getMutableNode(idxNode), nodeTemplate, rng);
            }
        }
    }
}