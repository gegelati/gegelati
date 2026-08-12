#include "node/genotypeTemplate.h"

size_t Node::GenotypeTemplate::size() const
{
    // NodeTemplates and NodeRanges sizes should always be equal, no security check for lowering complexity.
    return this->nodeTemplates.size();
}


void Node::GenotypeTemplate::addNodeTemplate(const Node::NodeTemplate& nodeTemplate, size_t minRange, size_t maxRange)
{
    this->nodeTemplates.push_back(nodeTemplate);

    // if Max range is 0 (default value), it is automatically set to minRange, to have a fix number of node for this template.
    if(maxRange == 0) {
        maxRange = minRange;
    }
    this->nodeRanges.push_back(std::make_pair(minRange, maxRange));
}


const Node::NodeTemplate& Node::GenotypeTemplate::getNodeTemplateAt(size_t idx) const
{
    if(idx >= this->nodeTemplates.size()) {
        throw std::runtime_error("Node:GenotypeTemplate:getNodeTemplateAt: index out of bounds.");
    }   
    return this->nodeTemplates.at(idx);
}


const std::pair<size_t, size_t>& Node::GenotypeTemplate::getRangeAt(size_t idx) const
{
    if(idx >= this->nodeRanges.size()) {
        throw std::runtime_error("Node:GenotypeTemplate:getNodeTemplateAt: index out of bounds.");
    }   
    return this->nodeRanges.at(idx);
}


const std::vector<Node::NodeTemplate>& Node::GenotypeTemplate::getNodeTemplates() const
{
    return this->nodeTemplates;
}


const std::vector<std::pair<size_t, size_t>>& Node::GenotypeTemplate::getRanges() const
{
    return this->nodeRanges;
}