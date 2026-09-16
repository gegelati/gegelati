#include "mutation/mutator.h"

std::unique_ptr<GraphBased::GPNode> Mutation::Mutator::createRandomNode(GraphBased::NodeGenerator& nodeGenerator, RNG::RNG& rng) const
{
    if(nodeGenerator.size() == 0) {
        throw std::runtime_error("Mutation::Mutator::createRandomNode: NodeGenerator is empty.");
    }

    std::vector<Data::DataValue> values;
    for(size_t idxValue = 0; idxValue < nodeGenerator.size(); idxValue++) {
        values.push_back(nodeGenerator.getGeneratorAt(idxValue).cloneUniquePtr()->sample(rng));
    }
    return std::make_unique<GraphBased::GPNode>(values);
}


std::unique_ptr<GraphBased::Genotype> Mutation::Mutator::initRandomGenotype(std::unique_ptr<GraphBased::GenotypeGenerator> genotypeGenerator, RNG::RNG& rng) const
{
    if(genotypeGenerator->size() == 0) {
        throw std::runtime_error("Mutation::Mutator::initRandomGenotype: genotypeGenerator is empty.");
    }
    std::unique_ptr<GraphBased::Genotype> genotype = std::make_unique<GraphBased::Genotype>();

    for(size_t idxGenerator = 0; idxGenerator < genotypeGenerator->size(); idxGenerator++) {
        std::unique_ptr<GraphBased::NodeGroup> nodeGroup = std::make_unique<GraphBased::NodeGroup>();

        const std::pair<size_t, size_t>& range = genotypeGenerator->getRangeAt(idxGenerator);
        size_t nbNodesOfGenerator = rng.getUnsignedInt64(range.first, range.second);

        for(size_t idxNode = 0; idxNode < nbNodesOfGenerator; idxNode++) {
            nodeGroup->addNode(this->createRandomNode(genotypeGenerator->getNodeGeneratorAt(idxGenerator), rng));
        }
        genotype->addNodeGroup(std::move(nodeGroup));
    }
    return std::move(genotype);
}