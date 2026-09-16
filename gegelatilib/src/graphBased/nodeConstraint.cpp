#include "graphBased/nodeConstraint.h"
            
            
void GraphBased::NodeConstraint::addConstraint(std::shared_ptr<const Dimensions::Constraint> constraint)
{
    this->constraints.push_back(constraint);
}

void GraphBased::NodeConstraint::addConstraint(const Dimensions::Constraint& constraint)
{
    this->addConstraint(constraint.cloneSharedPtr());
}

const Dimensions::Constraint& GraphBased::NodeConstraint::getConstraintAt(size_t idxValue) const
{
    if(idxValue >= this->constraints.size()) {
        throw std::runtime_error("Node:NodeRequirements:getRequirementAt: index out of bounds.");
    }
    return *this->constraints.at(idxValue);
}

size_t GraphBased::NodeConstraint::size() const 
{
    return this->constraints.size();
}

std::unique_ptr<GraphBased::NodeConstraint> GraphBased::NodeConstraint::cloneUniquePtr() const
{
    std::unique_ptr<NodeConstraint> clone = std::make_unique<NodeConstraint>();
    for(size_t idx = 0; idx < this->constraints.size(); idx++) {
        clone->addConstraint(this->constraints.at(idx));
    }
    return std::move(clone);
}