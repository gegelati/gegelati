#include "node/nodeGroup.h"
#include "evolution/individual.h"


void Node::NodeGroup::addNode(std::unique_ptr<GPNode> node, size_t index)
{
    if(index > this->nodes.size()){
        throw std::runtime_error("Node::NodeGroup::addNode: index out of range.");
    }
    this->nodes.insert(this->nodes.begin() + index, std::move(node));
}

void Node::NodeGroup::addNode(std::unique_ptr<GPNode> node)
{
    this->addNode(std::move(node), this->nodes.size());
}

void Node::NodeGroup::removeNode(size_t index)
{
    if(index >= this->nodes.size()){
        throw std::runtime_error("Node::NodeGroup::removeNode: index out of range.");
    }
    this->nodes.erase(this->nodes.begin() + index);
}

Node::GPNode& Node::NodeGroup::getMutableNode(size_t index)
{
    if(index >= this->getSize()){
        throw std::runtime_error("Node::NodeGroup::getMutableNode: index out of range.");
    }
    return *this->nodes[index];
}

const Node::GPNode& Node::NodeGroup::getNode(size_t index) const
{
    if(index >= this->getSize()){
        throw std::runtime_error("Node::NodeGroup::getNode: index out of range.");
    }
    return *this->nodes[index];
}

size_t Node::NodeGroup::getSize() const
{
    return this->nodes.size();
}

std::vector<std::reference_wrapper<const Node::GPNode>> Node::NodeGroup::getNodes() const
{
    std::vector<std::reference_wrapper<const GPNode>> result;
    result.reserve(this->nodes.size());

    for (const std::unique_ptr<GPNode>& node : this->nodes) {
        result.emplace_back(*node);
    }

    return result;
}

bool Node::operator==(const Node::NodeGroup& group1, const Node::NodeGroup& group2)
{
    if(group1.getSize() != group2.getSize()) {
        return false;
    }

    for(size_t idxNode = 0; idxNode < group1.getSize(); idxNode++) {
        const Node::GPNode& node1 = group1.getNode(idxNode);
        const Node::GPNode& node2 = group2.getNode(idxNode);

        if(node1.getSize() != node2.getSize()) {
            return false;
        }

        for(size_t idxValue = 0; idxValue < node1.getSize(); idxValue++) {
            if(node1.getValue(idxValue) != node2.getValue(idxValue)) {
                return false;
            }
        }
    }
    return true;
}

bool Node::operator!=(const Node::NodeGroup& group1, const Node::NodeGroup& group2)
{
    return !(group1 == group2);
}