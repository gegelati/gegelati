
#include "representation/agent.h"

// Declaration of static agent ID Counter in local here because it creates
// error in the .h file for MSVC compiler See:
// https://discourse.cmake.org/t/exporting-a-static-data-member-of-a-class-for-dll-using-msvc/5892
static uint64_t AGENT_COUNTER_ID = 0;

uint64_t Representation::Agent::incrementeCounter()
{
    return AGENT_COUNTER_ID++;
}

uint64_t Representation::Agent::getAgentIDCounter()
{
    return AGENT_COUNTER_ID;
}

void Representation::Agent::resetAgentIDCounter()
{
    AGENT_COUNTER_ID = 0;
}

uint64_t Representation::Agent::getAgentID() const
{
    return this->agentID;
}

void Representation::Agent::setAgentID(uint64_t newID)
{
    this->agentID = newID;

    // Update the ID counter if needed
    if (newID >= AGENT_COUNTER_ID) {
        AGENT_COUNTER_ID = newID + 1;
    }
}

bool Representation::operator<(const Representation::Agent& a, const Representation::Agent& b)
{
    return a.getAgentID() < b.getAgentID();
}

bool Representation::operator==(const Representation::Agent& a, const Representation::Agent& b)
{
    return a.getAgentID() == b.getAgentID();
}
bool Representation::operator!=(const Representation::Agent& a, const Representation::Agent& b)
{
    return a.getAgentID() != b.getAgentID();
}