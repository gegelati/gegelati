#include "node/genotypeTemplate.h"

size_t Node::GenotypeTemplate::size() const
{
    // NodeTemplate and NodeRanges sizes should always be equal, no security check for lowering complexity.
    return this->nodeTemplates.size();
}


void Node::GenotypeTemplate::addNodeTemplate(const NodeTemplate& nodeTemplates, size_t minRange, size_t maxRange)
{
    if(nodeTemplates.size() == 0) {
        throw std::runtime_error("Node:GenotypeTemplate:addNodeTemplate: nodeTemplates is empty.");
    }
    this->nodeTemplates.push_back(std::move(nodeTemplates.cloneUniquePtr()));

    // if Max range is 0 (default value), it is automatically set to minRange, to have a fix number of node for this template.
    if(maxRange == 0) {
        maxRange = minRange;
    }
    this->nodeRanges.push_back(std::make_pair(minRange, maxRange));
}


Node::NodeTemplate& Node::GenotypeTemplate::getNodeTemplateAt(size_t idx)
{
    if(idx >= this->nodeTemplates.size()) {
        throw std::runtime_error("Node:GenotypeTemplate:getNodeTemplateAt: index out of bounds.");
    }   
    return *this->nodeTemplates.at(idx);
}


const std::pair<size_t, size_t>& Node::GenotypeTemplate::getRangeAt(size_t idx) const
{
    if(idx >= this->nodeRanges.size()) {
        throw std::runtime_error("Node:GenotypeTemplate:getNodeTemplateAt: index out of bounds.");
    }   
    return this->nodeRanges.at(idx);
}

std::unique_ptr<Node::GenotypeTemplate> Node::GenotypeTemplate::cloneUniquePtr() const {
    std::unique_ptr<GenotypeTemplate> clone = std::make_unique<GenotypeTemplate>();
    for(size_t idx = 0; idx < this->nodeTemplates.size(); idx++) {
        const std::pair<size_t, size_t>& pairRef = this->nodeRanges.at(idx);
        clone->addNodeTemplate(*this->nodeTemplates.at(idx), pairRef.first, pairRef.second);
    }
    return std::move(clone);
}