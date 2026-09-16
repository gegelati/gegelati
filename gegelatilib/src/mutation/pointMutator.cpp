#include "mutation/pointMutator.h"


void Mutation::PointMutator::mutateNode(GraphBased::GPNode& node, GraphBased::NodeGenerator& nodeGenerator, RNG::RNG& rng) const
{
    if(nodeGenerator.size() != node.getSize()) {
        throw std::runtime_error("Mutation::PointMutator::mutateNode: NodeGenerator size does not correspond to the genotypeidual.");
    }
    // Sample random index
    size_t idxValueMutated = rng.getUnsignedInt64(0, node.getSize() - 1);

    // Sample random value for index
    Data::DataValue newValue = nodeGenerator.getGeneratorAt(idxValueMutated).sample(rng);

    // Copy node with assigned value
    node.setValue(idxValueMutated, newValue);
}


std::unique_ptr<GraphBased::Genotype> Mutation::PointMutator::mutateGenotype(const GraphBased::Genotype& genotype, std::unique_ptr<GraphBased::GenotypeGenerator> genotypeGenerator, RNG::RNG& rng) const
{
    if(genotypeGenerator->size() == 0) {
        throw std::runtime_error("Mutation::PointMutator::mutateGenotype: genotypeGenerator is empty.");
    }
    std::unique_ptr<GraphBased::Genotype> mutatedGenotype = genotype.cloneUniquePtr();

    for(size_t idxNodeGroup = 0; idxNodeGroup < genotype.getSize(); idxNodeGroup++) {
        const std::pair<size_t, size_t>& rangeGroup = genotypeGenerator->getRangeAt(idxNodeGroup);
        GraphBased::NodeGenerator& nodeGenerator = genotypeGenerator->getNodeGeneratorAt(idxNodeGroup);
        GraphBased::NodeGroup& mutatedGroup = mutatedGenotype->getNodeGroup(idxNodeGroup);
        
        /// Remove random node
        if(mutatedGroup.getSize() > rangeGroup.first && rng.getDouble(0, 1) < this->pRemoveNode) {
            mutatedGroup.removeNode(rng.uniformSample<size_t>(0, mutatedGroup.getSize() - 1));
        }

        /// Add random node
        if(mutatedGroup.getSize() < rangeGroup.second && rng.getDouble(0, 1) < this->pAddNode) {
            mutatedGroup.addNode(this->createRandomNode(nodeGenerator, rng));
        }
        /// Mutate nodes
        for(size_t idxNode = 0; idxNode < mutatedGroup.getSize(); idxNode++) {
            if(rng.getDouble(0, 1) < this->pMutateNode) {
                this->mutateNode(mutatedGroup.getNode(idxNode), nodeGenerator, rng);
            }
        }
    }
    return std::move(mutatedGenotype);
}