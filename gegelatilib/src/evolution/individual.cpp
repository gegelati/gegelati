
#include "evolution/individual.h"

// Declaration of static individual ID Counter in local here because it creates
// error in the .h file for MSVC compiler See:
// https://discourse.cmake.org/t/exporting-a-static-data-member-of-a-class-for-dll-using-msvc/5892
static size_t INDIVIDUAL_COUNTER_ID = 0;

size_t Evolution::Individual::incrementeCounter()
{
    return INDIVIDUAL_COUNTER_ID++;
}

size_t Evolution::Individual::getIndividualIDCounter()
{
    return INDIVIDUAL_COUNTER_ID;
}

void Evolution::Individual::resetIndividualIDCounter()
{
    INDIVIDUAL_COUNTER_ID = 0;
}

size_t Evolution::Individual::getIndividualID() const
{
    return this->individualID;
}

void Evolution::Individual::setIndividualID(size_t newID)
{
    this->individualID = newID;

    // Update the ID counter if needed
    if (newID >= INDIVIDUAL_COUNTER_ID) {
        INDIVIDUAL_COUNTER_ID = newID + 1;
    }
}


void Evolution::Individual::addGPNode(std::unique_ptr<Node::GPNode> node, size_t index, bool isIntron)
{
    if(index > this->genotype.size()){
        throw std::runtime_error("Evolution::Individual::addGPNode: index out of range.");
    }
    this->genotype.insert(this->genotype.begin() + index, std::move(node));
    this->isIntronNode.insert(this->isIntronNode.begin() + index, isIntron);
}

void Evolution::Individual::addGPNode(std::unique_ptr<Node::GPNode> node, bool isIntron)
{
    this->addGPNode(std::move(node), this->genotype.size(), isIntron);
}

void Evolution::Individual::removeGPNode(size_t index)
{
    if(index >= this->genotype.size()){
        throw std::runtime_error("Evolution::Individual::removeGPNode: index out of range.");
    }
    this->genotype.erase(this->genotype.begin() + index);
}

Node::GPNode& Evolution::Individual::getMutableGPNode(size_t index)
{
    if(index >= this->getSize()){
        throw std::runtime_error("Evolution::Individual::getMutableGPNode: index out of range.");
    }
    return *this->genotype[index];
}

const Node::GPNode& Evolution::Individual::getGPNode(size_t index) const
{
    if(index >= this->getSize()){
        throw std::runtime_error("Evolution::Individual::getGPNode: index out of range.");
    }
    return *this->genotype[index];
}

size_t Evolution::Individual::getSize() const
{
    return this->genotype.size();
}

std::vector<std::reference_wrapper<const Node::GPNode>> Evolution::Individual::getGenotype() const
{
    std::vector<std::reference_wrapper<const Node::GPNode>> result;
    result.reserve(genotype.size());

    for (const auto& node : genotype) {
        result.emplace_back(*node);
    }

    return result;
}

std::vector<std::reference_wrapper<const Node::GPNode>> Evolution::Individual::getEffectiveGenotype() const
{
    std::vector<std::reference_wrapper<const Node::GPNode>> result;

    for(size_t idx = 0; idx < this->getSize(); idx++) {
        if(!this->getIsIntronNode(idx)) {
            result.emplace_back(this->getGPNode(idx));
        }
    }

    return result;
}

void Evolution::Individual::setIsIntronNode(size_t index, bool isIntron)
{
    if(index >= this->getSize()){
        throw std::runtime_error("Evolution::Individual::setIsIntronNode: index out of range.");
    }
    this->isIntronNode[index] = isIntron;
}

bool Evolution::Individual::getIsIntronNode(size_t index) const
{
    if(index >= this->getSize()){
        throw std::runtime_error("Evolution::Individual::getIsIntronNode: index out of range.");
    }
    return this->isIntronNode[index];
}

const std::vector<bool>& Evolution::Individual::getAreIntronNodes() const
{
    return this->isIntronNode;
}



bool Evolution::operator<(const Evolution::Individual& a, const Evolution::Individual& b)
{
    return a.getIndividualID() < b.getIndividualID();
}

bool Evolution::operator==(const Evolution::Individual& a, const Evolution::Individual& b)
{
    return a.getIndividualID() == b.getIndividualID();
}
bool Evolution::operator!=(const Evolution::Individual& a, const Evolution::Individual& b)
{
    return !(a==b);
}
bool Evolution::operator>(const Evolution::Individual& a, const Evolution::Individual& b)
{
    return b < a;
}
bool Evolution::operator<=(const Evolution::Individual& a, const Evolution::Individual& b)
{
    return (a < b) || (a == b);
}
bool Evolution::operator>=(const Evolution::Individual& a, const Evolution::Individual& b)
{
    return (a > b) || (a == b);
}
