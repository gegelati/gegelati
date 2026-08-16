
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
        
/*bool Evolution::Individual::hasSameGenotypeAs(const Individual& other, bool effectiveGenotype) const
{
    auto thisGenotype = (effectiveGenotype) ? this->getEffectiveGenotype() : this->getGenotype();
    auto otherGenotype = (effectiveGenotype) ? other.getEffectiveGenotype() : other.getGenotype();
    if(thisGenotype.size() != otherGenotype.size()) {
        return false;
    }

    for(size_t idxNode = 0; idxNode < thisGenotype.size(); idxNode++) {
        const Node::GPNode& thisNode = thisGenotype.at(idxNode);
        const Node::GPNode& otherNode = otherGenotype.at(idxNode);

        if(thisNode.getSize() != otherNode.getSize()) {
            return false;
        }

        for(size_t idxValue = 0; idxValue < thisNode.getSize(); idxValue++) {
            if(thisNode.getValue(idxValue) != otherNode.getValue(idxValue)) {
                return false;
            }
        }
    }
    return true;
}*/



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
