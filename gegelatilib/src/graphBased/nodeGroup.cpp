#include "graphBased/nodeGroup.h"
#include "evolution/individual.h"



std::unique_ptr<GraphBased::NodeGroup> GraphBased::NodeGroup::cloneUniquePtr() const
{
    std::unique_ptr<GraphBased::NodeGroup> clone = std::make_unique<NodeGroup>();
    for(const std::unique_ptr<GraphBased::GPNode>& node : this->nodes) {
        clone->addNode(std::move(node->cloneUniquePtr()));
    }
    return std::move(clone);
}


void GraphBased::NodeGroup::addNode(std::unique_ptr<GPNode> node, size_t index)
{
    if(index > this->nodes.size()){
        throw std::runtime_error("GraphBased::NodeGroup::addNode: index out of range.");
    }
    this->nodes.insert(this->nodes.begin() + index, std::move(node));
}

void GraphBased::NodeGroup::addNode(std::unique_ptr<GPNode> node)
{
    this->addNode(std::move(node), this->nodes.size());
}

void GraphBased::NodeGroup::removeNode(size_t index)
{
    if(index >= this->nodes.size()){
        throw std::runtime_error("GraphBased::NodeGroup::removeNode: index out of range.");
    }
    this->nodes.erase(this->nodes.begin() + index);
}

void GraphBased::NodeGroup::setNode(std::unique_ptr<GPNode> node, size_t index)
{
    if(index >= this->getSize()){
        throw std::runtime_error("GraphBased::NodeGroup::getMutableNode: index out of range.");
    }
    this->nodes[index] = std::move(node);
}

const GraphBased::GPNode& GraphBased::NodeGroup::getNode(size_t index) const
{
    if(index >= this->getSize()){
        throw std::runtime_error("GraphBased::NodeGroup::getNode: index out of range.");
    }
    return *this->nodes[index];
}

GraphBased::GPNode& GraphBased::NodeGroup::getNode(size_t index)
{
    if(index >= this->getSize()){
        throw std::runtime_error("GraphBased::NodeGroup::getNode: index out of range.");
    }
    return *this->nodes[index];
}

size_t GraphBased::NodeGroup::getSize() const
{
    return this->nodes.size();
}

std::vector<std::reference_wrapper<const GraphBased::GPNode>> GraphBased::NodeGroup::getNodes() const
{
    std::vector<std::reference_wrapper<const GPNode>> result;
    result.reserve(this->nodes.size());

    for (const std::unique_ptr<GPNode>& node : this->nodes) {
        result.emplace_back(*node);
    }

    return result;
}

bool GraphBased::operator==(const GraphBased::NodeGroup& group1, const GraphBased::NodeGroup& group2)
{
    if(group1.getSize() != group2.getSize()) {
        return false;
    }

    for(size_t idxNode = 0; idxNode < group1.getSize(); idxNode++) {
        if(!group1.getNode(idxNode).hasSameValues(group2.getNode(idxNode))) {
            return false;
        }
    }
    return true;
}

bool GraphBased::operator!=(const GraphBased::NodeGroup& group1, const GraphBased::NodeGroup& group2)
{
    return !(group1 == group2);
}