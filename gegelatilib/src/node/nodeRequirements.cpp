#include "node/nodeRequirements.h"
            
            
void Node::NodeRequirements::addValueRequirements(const NodeValueRequirement& nodeValueRequirement)
{
    if(nodeValueRequirement.size() == 0) {
        throw std::runtime_error("Node:NodeRequirements:addValueRequirements: Cannot add empty nodeValueRequirement.");
    }
    this->nodeValueRequirements.push_back(nodeValueRequirement);
}

const std::vector<Node::NodeValueRequirement>& Node::NodeRequirements::getValueRequirements() const
{
    return this->nodeValueRequirements;
}
const Node::NodeValueRequirement& Node::NodeRequirements::getValueRequirementsAt(size_t idxValue) const
{
    if(idxValue >= this->nodeValueRequirements.size()) {
        throw std::runtime_error("Node:NodeRequirements:getValueRequirementsAt: index out of bounds.");
    }
    return this->nodeValueRequirements.at(idxValue);
}

size_t Node::NodeRequirements::size() const 
{
    return this->nodeValueRequirements.size();
}