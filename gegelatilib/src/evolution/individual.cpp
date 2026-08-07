
#include "evolution/individual.h"

// Declaration of static individual ID Counter in local here because it creates
// error in the .h file for MSVC compiler See:
// https://discourse.cmake.org/t/exporting-a-static-data-member-of-a-class-for-dll-using-msvc/5892
static uint64_t INDIVIDUAL_COUNTER_ID = 0;

uint64_t Evolution::Individual::incrementeCounter()
{
    return INDIVIDUAL_COUNTER_ID++;
}

uint64_t Evolution::Individual::getIndividualIDCounter()
{
    return INDIVIDUAL_COUNTER_ID;
}

void Evolution::Individual::resetIndividualIDCounter()
{
    INDIVIDUAL_COUNTER_ID = 0;
}

uint64_t Evolution::Individual::getIndividualID() const
{
    return this->individualID;
}

void Evolution::Individual::setIndividualID(uint64_t newID)
{
    this->individualID = newID;

    // Update the ID counter if needed
    if (newID >= INDIVIDUAL_COUNTER_ID) {
        INDIVIDUAL_COUNTER_ID = newID + 1;
    }
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
    return a.getIndividualID() != b.getIndividualID();
}