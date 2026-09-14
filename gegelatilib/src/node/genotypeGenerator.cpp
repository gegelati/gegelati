#include "node/genotypeGenerator.h"

size_t Node::GenotypeGenerator::size() const
{
    // NodeGenerator and NodeRanges sizes should always be equal, no security check for lowering complexity.
    return this->nodeGenerators.size();
}


void Node::GenotypeGenerator::addNodeGenerator(std::unique_ptr<NodeGenerator> nodeGenerator, size_t minRange, size_t maxRange)
{
    if(nodeGenerator->size() == 0) {
        throw std::runtime_error("Node:GenotypeGenerator:addNodeGenerator: nodeGenerators is empty.");
    }
    this->nodeGenerators.push_back(std::move(nodeGenerator));

    // if Max range is 0 (default value), it is automatically set to minRange, to have a fix number of node for this generator.
    if(maxRange == 0) {
        maxRange = minRange;
    }
    this->nodeRanges.push_back(std::make_pair(minRange, maxRange));
}

void Node::GenotypeGenerator::addNodeGenerator(const NodeGenerator& nodeGenerator, size_t minRange, size_t maxRange)
{
    this->addNodeGenerator(std::move(nodeGenerator.cloneUniquePtr()), minRange, maxRange);
}


Node::NodeGenerator& Node::GenotypeGenerator::getNodeGeneratorAt(size_t idx)
{
    if(idx >= this->nodeGenerators.size()) {
        throw std::runtime_error("Node:GenotypeGenerator:getNodeGeneratorAt: index out of bounds.");
    }   
    return *this->nodeGenerators.at(idx);
}


const std::pair<size_t, size_t>& Node::GenotypeGenerator::getRangeAt(size_t idx) const
{
    if(idx >= this->nodeRanges.size()) {
        throw std::runtime_error("Node:GenotypeGenerator:getNodeGeneratorAt: index out of bounds.");
    }   
    return this->nodeRanges.at(idx);
}

std::unique_ptr<Node::GenotypeGenerator> Node::GenotypeGenerator::cloneUniquePtr() const {
    std::unique_ptr<GenotypeGenerator> clone = std::make_unique<GenotypeGenerator>();
    for(size_t idx = 0; idx < this->nodeGenerators.size(); idx++) {
        const std::pair<size_t, size_t>& pairRef = this->nodeRanges.at(idx);
        clone->addNodeGenerator(*this->nodeGenerators.at(idx), pairRef.first, pairRef.second);
    }
    return std::move(clone);
}