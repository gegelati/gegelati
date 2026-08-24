
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

size_t Evolution::Individual::getSize() const
{
    return this->genotype->getFullSize();
}

const Evolution::Genotype& Evolution::Individual::getGenotype() const
{
    return *this->genotype;
}

Evolution::Genotype& Evolution::Individual::getMutableGenotype()
{
    return *this->genotype;
}
        

std::unique_ptr<Evolution::Individual> Evolution::Individual::cloneUniquePtr() const
{
    std::unique_ptr<Individual> newIndividual = std::make_unique<Individual>();

    for(const Node::NodeGroup& group: this->genotype->getNodeGroups()) {
        Node::NodeGroup& newNodeGroup = newIndividual->genotype->addNodeGroup();
        
        for(const Node::GPNode& node: group.getNodes()) {
            newNodeGroup.addNode(std::make_unique<Node::GPNode>(node.getValues()));
        }
    }

    return std::move(newIndividual);
}

void Evolution::Individual::addEvaluationRun(std::unique_ptr<Evaluation::EvaluationRun> evaluationRun, size_t seed) const
{
    this->result->addEvaluationRun(std::move(evaluationRun), seed);
}

const Evaluation::EvaluationResult& Evolution::Individual::getEvaluationResult() const
{
    return *this->result;
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
