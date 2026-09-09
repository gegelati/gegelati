#include "node/nodeTemplate.h"
            
            
void Node::NodeTemplate::addTemplate(const Dimensions::Constraint& constraint, const Dimensions::DataValueGenerator& generator)
{
    this->constraints.push_back(constraint.cloneSharedPtr());
    this->generators.push_back(generator.cloneUniquePtr());
}

const Dimensions::Constraint& Node::NodeTemplate::getConstraintAt(size_t idxValue) const
{
    if(idxValue >= this->constraints.size()) {
        throw std::runtime_error("Node:NodeRequirements:getRequirementAt: index out of bounds.");
    }
    return *this->constraints.at(idxValue);
}
Dimensions::DataValueGenerator& Node::NodeTemplate::getGeneratorAt(size_t idxValue)
{
    if(idxValue >= this->generators.size()) {
        throw std::runtime_error("Node:NodeRequirements:getRequirementAt: index out of bounds.");
    }
    return *this->generators.at(idxValue);
}

size_t Node::NodeTemplate::size() const 
{
    return this->constraints.size();
}



std::unique_ptr<Node::NodeTemplate> Node::NodeTemplate::cloneUniquePtr() const
{
    std::unique_ptr<NodeTemplate> clone = std::make_unique<NodeTemplate>();
    for(size_t idx = 0; idx < this->constraints.size(); idx++) {
        clone->addTemplate(*this->constraints.at(idx), *this->generators.at(idx));
    }
    return std::move(clone);
}