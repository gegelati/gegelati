#include "node/nodeConstraint.h"
            
            
void Node::NodeConstraint::addConstraint(std::shared_ptr<const Dimensions::Constraint> constraint)
{
    this->constraints.push_back(constraint);
}

void Node::NodeConstraint::addConstraint(const Dimensions::Constraint& constraint)
{
    this->addConstraint(constraint.cloneSharedPtr());
}

const Dimensions::Constraint& Node::NodeConstraint::getConstraintAt(size_t idxValue) const
{
    if(idxValue >= this->constraints.size()) {
        throw std::runtime_error("Node:NodeRequirements:getRequirementAt: index out of bounds.");
    }
    return *this->constraints.at(idxValue);
}

size_t Node::NodeConstraint::size() const 
{
    return this->constraints.size();
}

std::unique_ptr<Node::NodeConstraint> Node::NodeConstraint::cloneUniquePtr() const
{
    std::unique_ptr<NodeConstraint> clone = std::make_unique<NodeConstraint>();
    for(size_t idx = 0; idx < this->constraints.size(); idx++) {
        clone->addConstraint(this->constraints.at(idx));
    }
    return std::move(clone);
}