#include "evolution/genotype.h"
#include "evolution/individual.h"

void Evolution::Genotype::addNodeGroup(size_t index)
{
    if(index > this->nodeGroups.size()){
        throw std::runtime_error("Evolution::Genotype::addNodeGroup: index out of range.");
    }
    this->nodeGroups.insert(this->nodeGroups.begin() + index, std::make_unique<Node::NodeGroup>());

}

void Evolution::Genotype::addNodeGroup()
{
    this->addNodeGroup(this->nodeGroups.size());
}

void Evolution::Genotype::removeNodeGroup(size_t index)
{
    if(index >= this->nodeGroups.size()){
        throw std::runtime_error("Evolution::Individual::removeNodeGroup: index out of range.");
    }
    this->nodeGroups.erase(this->nodeGroups.begin() + index);

}

const Node::NodeGroup& Evolution::Genotype::getNodeGroup(size_t index) const
{
    if(index >= this->nodeGroups.size()){
        throw std::runtime_error("Evolution::Individual::getNodeGroup: index out of range.");
    }
    return *this->nodeGroups[index];

}

Node::NodeGroup& Evolution::Genotype::getMutableNodeGroup(size_t index)
{
    if(index >= this->nodeGroups.size()){
        throw std::runtime_error("Evolution::Individual::getMutableNodeGroup: index out of range.");
    }
    return *this->nodeGroups[index];

}

size_t Evolution::Genotype::getSize() const
{
    return this->nodeGroups.size();
}

size_t Evolution::Genotype::getFullSize() const
{
    size_t size = 0;
    for(const std::unique_ptr<Node::NodeGroup>& nodeGroup: this->nodeGroups) {
        size += nodeGroup->getSize();
    }
    return size;
}

std::vector<size_t> Evolution::Genotype::getSizes() const
{
    std::vector<size_t> sizes;
    for(const std::unique_ptr<Node::NodeGroup>& nodeGroup: this->nodeGroups) {
        sizes.push_back(nodeGroup->getSize());
    }
    return sizes;
}

std::vector<std::reference_wrapper<const Node::NodeGroup>> Evolution::Genotype::getNodeGroups() const
{
    std::vector<std::reference_wrapper<const Node::NodeGroup>> result;
    result.reserve(this->nodeGroups.size());

    for (const std::unique_ptr<Node::NodeGroup>& nodeGroup : this->nodeGroups) {
        result.emplace_back(*nodeGroup);
    }

    return result;

}

std::vector<std::vector<std::reference_wrapper<const Node::GPNode>>> Evolution::Genotype::getEffectiveNodes() const
{
    std::vector<std::vector<std::reference_wrapper<const Node::GPNode>>> result;

    for (const std::unique_ptr<Node::NodeGroup>& nodeGroup : this->nodeGroups) {
        std::vector<std::reference_wrapper<const Node::GPNode>> effectiveNodeGoup;
        for (size_t idx = 0; idx < nodeGroup->getSize(); idx++) {
            const Node::GPNode& node = nodeGroup->getNode(idx);
            if(!node.getIsIntron()) {
                effectiveNodeGoup.push_back(node);
            }
        }

        result.emplace_back(effectiveNodeGoup);
    }

    return result;

}

bool Evolution::operator==(const Genotype& genotype1, const Genotype& genotype2)
{
    if(genotype1.getSize() != genotype2.getSize()) {
        return false;
    }

    for(size_t idxGroup = 0; idxGroup < genotype1.getSize(); idxGroup++) {
        if(genotype1.getNodeGroup(idxGroup) != genotype2.getNodeGroup(idxGroup)){
            return false;
        }
    }
    return true;
}

bool Evolution::operator!=(const Genotype& genotype1, const Genotype& genotype2)
{
    return !(genotype1 == genotype2);
}