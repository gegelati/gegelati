
#include "representation/individual.h"

// Declaration of static individual ID Counter in local here because it creates
// error in the .h file for MSVC compiler See:
// https://discourse.cmake.org/t/exporting-a-static-data-member-of-a-class-for-dll-using-msvc/5892
static uint64_t INDIVIDUAL_COUNTER_ID = 0;

uint64_t Representation::Individual::incrementeCounter()
{
    return INDIVIDUAL_COUNTER_ID++;
}

uint64_t Representation::Individual::getIndividualIDCounter()
{
    return INDIVIDUAL_COUNTER_ID;
}

void Representation::Individual::resetIndividualIDCounter()
{
    INDIVIDUAL_COUNTER_ID = 0;
}

uint64_t Representation::Individual::getIndividualID() const
{
    return this->individualID;
}

void Representation::Individual::setIndividualID(uint64_t newID)
{
    this->individualID = newID;

    // Update the ID counter if needed
    if (newID >= INDIVIDUAL_COUNTER_ID) {
        INDIVIDUAL_COUNTER_ID = newID + 1;
    }
}

bool Representation::operator<(const Representation::Individual& a, const Representation::Individual& b)
{
    return a.getIndividualID() < b.getIndividualID();
}

bool Representation::operator==(const Representation::Individual& a, const Representation::Individual& b)
{
    return a.getIndividualID() == b.getIndividualID();
}
bool Representation::operator!=(const Representation::Individual& a, const Representation::Individual& b)
{
    return a.getIndividualID() != b.getIndividualID();
}