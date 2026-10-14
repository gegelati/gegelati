#include "mutation/pointMutator.h"


void Mutation::PointMutator::mutateNode(GraphBased::GPNode& node, const GraphBased::NodeGenerator& nodeGenerator, RNG::RNG& rng) const
{
    if(nodeGenerator.size() != node.getSize()) {
        throw std::runtime_error("Mutation::PointMutator::mutateNode: NodeGenerator size does not correspond to the genotype.");
    }
    // Sample random index
    size_t idxValueMutated = rng.getUnsignedInt64(0, node.getSize() - 1);


    // Mutate the value using its current value and the configured generator.
    Data::DataValue newValue = nodeGenerator.getGeneratorAt(idxValueMutated)
        .mutate(node.getValue(idxValueMutated), rng);

    // Copy node with assigned value
    node.setValue(idxValueMutated, newValue);
}

void Mutation::PointMutator::addNode(
    GraphBased::NodeGroup &mutatedGroup, size_t idxNodeGroup, const GraphBased::NodeGenerator& nodeGenerator, RNG::RNG& rng) const
{
    mutatedGroup.addNode(this->createRandomNode(nodeGenerator, rng));
}

void Mutation::PointMutator::removeNode(
    GraphBased::NodeGroup &mutatedGroup, size_t idxNodeGroup, RNG::RNG& rng) const
{
    mutatedGroup.removeNode(rng.uniformSample<size_t>(0, mutatedGroup.getSize() - 1));
}


std::unique_ptr<GraphBased::Genotype> Mutation::PointMutator::mutateGenotype(const GraphBased::Genotype& genotype, const GraphBased::GenotypeGenerator& genotypeGenerator, RNG::RNG& rng) const
{
    if(genotypeGenerator.size() == 0) {
        throw std::runtime_error("Mutation::PointMutator::mutateGenotype: genotypeGenerator is empty.");
    }
    std::unique_ptr<GraphBased::Genotype> mutatedGenotype = genotype.cloneUniquePtr();

    for(size_t idxNodeGroup = 0; idxNodeGroup < genotype.getSize(); idxNodeGroup++) {
        const std::pair<size_t, size_t>& rangeGroup = genotypeGenerator.getMutationRangeAt(idxNodeGroup);
        const GraphBased::NodeGenerator& nodeGenerator = genotypeGenerator.getNodeGeneratorAt(idxNodeGroup);
        GraphBased::NodeGroup& mutatedGroup = mutatedGenotype->getNodeGroup(idxNodeGroup);
        
        /// Remove random node
        if(mutatedGroup.getSize() > rangeGroup.first && rng.getDouble(0, 1) < this->pRemoveNode) {
            this->removeNode(mutatedGroup, idxNodeGroup, rng);
        }

        /// Add random node
        if(mutatedGroup.getSize() < rangeGroup.second && rng.getDouble(0, 1) < this->pAddNode) {
            this->addNode(mutatedGroup, idxNodeGroup, nodeGenerator, rng);
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