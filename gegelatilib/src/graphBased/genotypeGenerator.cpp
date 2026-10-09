#include "graphBased/genotypeGenerator.h"

size_t GraphBased::GenotypeGenerator::size() const
{
    // NodeGenerator and NodeRanges sizes should always be equal, no security check for lowering complexity.
    return this->nodeGenerators.size();
}

void GraphBased::GenotypeGenerator::addNodeGenerator(
    std::unique_ptr<NodeGenerator> nodeGenerator, std::pair<size_t, size_t> mutationRange, std::pair<size_t, size_t>  initRange)
{
    if(nodeGenerator->size() == 0) {
        throw std::runtime_error("Node:GenotypeGenerator:addNodeGenerator: nodeGenerator is empty.");
    }
    this->nodeGenerators.push_back(std::move(nodeGenerator));

    this->nodeMutationRanges.push_back(mutationRange);

    if(initRange.first == 0 && initRange.second == 0) {
        this->nodeInitializeRanges.push_back(mutationRange);
    } {
        this->nodeInitializeRanges.push_back(initRange);
    }
}

void GraphBased::GenotypeGenerator::addNodeGenerator(
    const NodeGenerator& nodeGenerator, std::pair<size_t, size_t> mutationRange, std::pair<size_t, size_t>  initRange)
{
    this->addNodeGenerator(std::move(nodeGenerator.cloneUniquePtr()), mutationRange, initRange);
}

void GraphBased::GenotypeGenerator::addFixRangeNodeGenerator(std::unique_ptr<NodeGenerator> nodeGenerator, size_t fixRange)
{
    this->addNodeGenerator(std::move(nodeGenerator), std::make_pair(fixRange, fixRange));
}

void GraphBased::GenotypeGenerator::addFixRangeNodeGenerator(const NodeGenerator& nodeGenerator, size_t fixRange)
{
    this->addFixRangeNodeGenerator(std::move(nodeGenerator.cloneUniquePtr()), fixRange);
}



GraphBased::NodeGenerator& GraphBased::GenotypeGenerator::getNodeGeneratorAt(size_t idx)
{
    if(idx >= this->nodeGenerators.size()) {
        throw std::runtime_error("Node:GenotypeGenerator:getNodeGeneratorAt: index out of bounds.");
    }   
    return *this->nodeGenerators.at(idx);
}


const std::pair<size_t, size_t>& GraphBased::GenotypeGenerator::getMutationRangeAt(size_t idx) const
{
    if(idx >= this->nodeMutationRanges.size()) {
        throw std::runtime_error("Node:GenotypeGenerator:getMutationRangeAt: index out of bounds.");
    }   
    return this->nodeMutationRanges.at(idx);
}

const std::pair<size_t, size_t>& GraphBased::GenotypeGenerator::getInitRangeAt(size_t idx) const
{
    if(idx >= this->nodeInitializeRanges.size()) {
        throw std::runtime_error("Node:GenotypeGenerator:getInitRangeAt: index out of bounds.");
    }   
    return this->nodeInitializeRanges.at(idx);
}

std::unique_ptr<GraphBased::GenotypeGenerator> GraphBased::GenotypeGenerator::cloneUniquePtr() const {
    std::unique_ptr<GenotypeGenerator> clone = std::make_unique<GenotypeGenerator>();
    for(size_t idx = 0; idx < this->nodeGenerators.size(); idx++) {
        clone->addNodeGenerator(*this->nodeGenerators.at(idx), this->nodeMutationRanges.at(idx), this->nodeInitializeRanges.at(idx));
    }
    return std::move(clone);
}