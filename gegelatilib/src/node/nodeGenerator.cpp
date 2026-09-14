#include "node/nodeGenerator.h"
            
            
void Node::NodeGenerator::addGenerator(std::unique_ptr<Dimensions::DataValueGenerator> generator)
{
    this->generators.push_back(std::move(generator));
}
void Node::NodeGenerator::addGenerator(const Dimensions::DataValueGenerator& generator)
{
    this->addGenerator(std::move(generator.cloneUniquePtr()));
}

Dimensions::DataValueGenerator& Node::NodeGenerator::getGeneratorAt(size_t idxValue)
{
    if(idxValue >= this->generators.size()) {
        throw std::runtime_error("Node:NodeRequirements:getRequirementAt: index out of bounds.");
    }
    return *this->generators.at(idxValue);
}

size_t Node::NodeGenerator::size() const 
{
    return this->generators.size();
}

std::unique_ptr<Node::NodeGenerator> Node::NodeGenerator::cloneUniquePtr() const
{
    std::unique_ptr<NodeGenerator> clone = std::make_unique<NodeGenerator>();
    for(size_t idx = 0; idx < this->generators.size(); idx++) {
        clone->addGenerator(std::move(this->generators.at(idx)->cloneUniquePtr()));
    }
    return std::move(clone);
}