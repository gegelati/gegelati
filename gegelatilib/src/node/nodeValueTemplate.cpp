#include "node/nodeValueTemplate.h"

void Node::NodeValueTemplate::addValueConfiguration(const NodeValueConfiguration& configuration)
{
    this->configurations.push_back(configuration);
}

const std::vector<Node::NodeValueConfiguration>& Node::NodeValueTemplate::getconfigurations() const
{
    return this->configurations;
}

const Node::NodeValueConfiguration& Node::NodeValueTemplate::getconfigurationAt(size_t idxConfig) const
{
    if(idxConfig >= this->configurations.size()) {
        throw std::runtime_error("NodeValueTemplate::getconfigurationAt: index out of bounds.");
    }
    return this->configurations.at(idxConfig);
}

size_t Node::NodeValueTemplate::size() const
{
    return this->configurations.size();
}