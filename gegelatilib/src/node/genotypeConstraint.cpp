#include "node/genotypeConstraint.h"

size_t Node::GenotypeConstraint::size() const
{
    // NodeConstraint and NodeRanges sizes should always be equal, no security check for lowering complexity.
    return this->nodeConstraints.size();
}


void Node::GenotypeConstraint::addNodeConstraint(std::unique_ptr<NodeConstraint> nodeConstraint, size_t minRange, size_t maxRange)
{
    if(nodeConstraint->size() == 0) {
        throw std::runtime_error("Node:GenotypeConstraint:addNodeConstraint: nodeConstraints is empty.");
    }
    this->nodeConstraints.push_back(std::move(nodeConstraint));

    // if Max range is 0 (default value), it is automatically set to minRange, to have a fix number of node for this constraint.
    if(maxRange == 0) {
        maxRange = minRange;
    }
    this->nodeRanges.push_back(std::make_pair(minRange, maxRange));
}

void Node::GenotypeConstraint::addNodeConstraint(const NodeConstraint& nodeConstraint, size_t minRange, size_t maxRange)
{
    this->addNodeConstraint(std::move(nodeConstraint.cloneUniquePtr()), minRange, maxRange);
}


const Node::NodeConstraint& Node::GenotypeConstraint::getNodeConstraintAt(size_t idx) const
{
    if(idx >= this->nodeConstraints.size()) {
        throw std::runtime_error("Node:GenotypeConstraint:getNodeConstraintAt: index out of bounds.");
    }   
    return *this->nodeConstraints.at(idx);
}


const std::pair<size_t, size_t>& Node::GenotypeConstraint::getRangeAt(size_t idx) const
{
    if(idx >= this->nodeRanges.size()) {
        throw std::runtime_error("Node:GenotypeConstraint:getNodeConstraintAt: index out of bounds.");
    }   
    return this->nodeRanges.at(idx);
}

std::unique_ptr<Node::GenotypeConstraint> Node::GenotypeConstraint::cloneUniquePtr() const {
    std::unique_ptr<GenotypeConstraint> clone = std::make_unique<GenotypeConstraint>();
    for(size_t idx = 0; idx < this->nodeConstraints.size(); idx++) {
        const std::pair<size_t, size_t>& pairRef = this->nodeRanges.at(idx);
        clone->addNodeConstraint(*this->nodeConstraints.at(idx), pairRef.first, pairRef.second);
    }
    return std::move(clone);
}