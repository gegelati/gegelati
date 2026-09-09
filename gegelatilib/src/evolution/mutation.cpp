#include "evolution/mutation.h"

#include "evolution/individual.h"


std::unique_ptr<Node::GPNode> Evolution::Mutation::createRandomNode(Node::NodeTemplate& nodeTemplate, RNG::RNG& rng)
{
    if(nodeTemplate.size() == 0) {
        throw std::runtime_error("Evolution::Mutation::createRandomNode: NodeTemplate is empty.");
    }

    std::vector<Data::DataValue> values;
    for(size_t idxValue = 0; idxValue < nodeTemplate.size(); idxValue++) {
        values.push_back(nodeTemplate.getGeneratorAt(idxValue).cloneUniquePtr()->sample(rng));
    }
    return std::make_unique<Node::GPNode>(values);
}



void Evolution::Mutation::initRandomGenotype(Genotype& genotype, std::unique_ptr<Node::GenotypeTemplate> genotypeTemplate, RNG::RNG& rng)
{
    if(genotypeTemplate->size() == 0) {
        throw std::runtime_error("Evolution::Mutation::initRandomGenotype: genotypeTemplate is empty.");
    }

    for(size_t idxTemplate = 0; idxTemplate < genotypeTemplate->size(); idxTemplate++) {
        Node::NodeGroup& nodeGroup = genotype.addNodeGroup();
        
        const std::pair<size_t, size_t>& range = genotypeTemplate->getRangeAt(idxTemplate);
        size_t nbNodesOfTemplate = rng.getUnsignedInt64(range.first, range.second);

        for(size_t idxNode = 0; idxNode < nbNodesOfTemplate; idxNode++) {
            nodeGroup.addNode(this->createRandomNode(genotypeTemplate->getNodeTemplateAt(idxTemplate), rng));
        }
    }
}

void Evolution::Mutation::mutateNode(Node::GPNode& node, Node::NodeTemplate& nodeTemplate, RNG::RNG& rng)
{
    if(nodeTemplate.size() != node.getSize()) {
        throw std::runtime_error("Evolution::Mutation::mutateNode: NodeTemplate size does not correspond to the genotypeidual.");
    }
    size_t idxValueMutated;
    /*Data::DataValue newValue;
    // Simple loop to ensure one value is mutated.
    do {
        idxValueMutated = rng.getUnsignedInt64(0, node.getSize() - 1);
        newValue = this->sampleNodeValue(*nodeTemplate.getValueTemplateAt(idxValueMutated), rng);
    } while (node.getValue(idxValueMutated) == newValue);


    node.setValue(idxValueMutated, newValue);
    
    */
}


void Evolution::Mutation::mutateGenotype(Genotype& genotype, std::unique_ptr<Node::GenotypeTemplate> genotypeTemplate, RNG::RNG& rng)
{
    if(genotypeTemplate->size() == 0) {
        throw std::runtime_error("Evolution::Mutation::mutateGenotype: genotypeTemplate is empty.");
    }

    double pMutateNode = 0.5;

    Node::NodeTemplate& nodeTemplate = genotypeTemplate->getNodeTemplateAt(0);
    for(size_t idxNodeGroup = 0; idxNodeGroup < genotype.getSize(); idxNodeGroup++) {
        Node::NodeGroup& nodeGroup = genotype.getMutableNodeGroup(idxNodeGroup);

        for(size_t idxNode = 0; idxNode < nodeGroup.getSize(); idxNode++) {
            if(rng.getDouble(0, 1) < pMutateNode) {
                this->mutateNode(nodeGroup.getMutableNode(idxNode), nodeTemplate, rng);
            }
        }
    }
}