#include "node/nodeValueRequirement.h"

void Node::NodeValueRequirement::addRequirement(const Dimensions::Requirement& requirement)
{
    this->requirements.push_back(requirement);
}

const std::vector<Dimensions::Requirement>& Node::NodeValueRequirement::getRequirements() const
{
    return this->requirements;
}

const Dimensions::Requirement& Node::NodeValueRequirement::getRequirementAt(size_t idxConfig) const
{
    if(idxConfig >= this->requirements.size()) {
        throw std::runtime_error("NodeValueRequirement::getRequirementAt: index out of bounds.");
    }
    return this->requirements.at(idxConfig);
}

size_t Node::NodeValueRequirement::size() const
{
    return this->requirements.size();
}