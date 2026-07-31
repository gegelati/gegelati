
#include "representation/individual.h"

// Declaration of static agent ID Counter in local here because it creates
// error in the .h file for MSVC compiler See:
// https://discourse.cmake.org/t/exporting-a-static-data-member-of-a-class-for-dll-using-msvc/5892
static uint64_t AGENT_COUNTER_ID = 0;

uint64_t Representation::Individual::incrementeCounter()
{
    return AGENT_COUNTER_ID++;
}

uint64_t Representation::Individual::getAgentIDCounter()
{
    return AGENT_COUNTER_ID;
}

void Representation::Individual::resetAgentIDCounter()
{
    AGENT_COUNTER_ID = 0;
}

uint64_t Representation::Individual::getAgentID() const
{
    return this->agentID;
}

void Representation::Individual::setAgentID(uint64_t newID)
{
    this->agentID = newID;

    // Update the ID counter if needed
    if (newID >= AGENT_COUNTER_ID) {
        AGENT_COUNTER_ID = newID + 1;
    }
}

bool Representation::operator<(const Representation::Individual& a, const Representation::Individual& b)
{
    return a.getAgentID() < b.getAgentID();
}

bool Representation::operator==(const Representation::Individual& a, const Representation::Individual& b)
{
    return a.getAgentID() == b.getAgentID();
}
bool Representation::operator!=(const Representation::Individual& a, const Representation::Individual& b)
{
    return a.getAgentID() != b.getAgentID();
}