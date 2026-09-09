#include "node/genotypeRequirements.h"

size_t Node::GenotypeRequirements::size() const
{
    // NodeRequirements and NodeRanges sizes should always be equal, no security check for lowering complexity.
    return this->nodeRequirements.size();
}


void Node::GenotypeRequirements::addNodeRequirements(const NodeRequirements& nodeRequirements, size_t minRange, size_t maxRange)
{
    if(nodeRequirements.size() == 0) {
        throw std::runtime_error("Node:GenotypeRequirements:addNodeRequirements: nodeRequirements is empty.");
    }
    this->nodeRequirements.push_back(nodeRequirements);

    // if Max range is 0 (default value), it is automatically set to minRange, to have a fix number of node for this requirements.
    if(maxRange == 0) {
        maxRange = minRange;
    }
    this->nodeRanges.push_back(std::make_pair(minRange, maxRange));
}


const Node::NodeRequirements& Node::GenotypeRequirements::getNodeRequirementsAt(size_t idx) const
{
    if(idx >= this->nodeRequirements.size()) {
        throw std::runtime_error("Node:GenotypeRequirements:getNodeRequirementsAt: index out of bounds.");
    }   
    return this->nodeRequirements.at(idx);
}


const std::pair<size_t, size_t>& Node::GenotypeRequirements::getRangeAt(size_t idx) const
{
    if(idx >= this->nodeRanges.size()) {
        throw std::runtime_error("Node:GenotypeRequirements:getNodeRequirementsAt: index out of bounds.");
    }   
    return this->nodeRanges.at(idx);
}


const std::vector<Node::NodeRequirements>& Node::GenotypeRequirements::getNodeRequirements() const
{
    return this->nodeRequirements;
}


const std::vector<std::pair<size_t, size_t>>& Node::GenotypeRequirements::getRanges() const
{
    return this->nodeRanges;
}