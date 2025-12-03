

#include "algorithm/agentManager.h"


const std::set<std::shared_ptr<Algorithm::Agent>, SharedLess<Algorithm::Agent>>& Algorithm::AgentManager::getAgents()
{
    return this->agents;
}


const std::vector<std::shared_ptr<const Algorithm::Agent>> Algorithm::AgentManager::getAgentsCst() const
{
    std::vector<std::shared_ptr<const Algorithm::Agent>> constAgents;

    // Transform each element from shared_ptr<Agent> to shared_ptr<const Agent>
    std::transform(
        this->agents.begin(),
        this->agents.end(),
        std::back_inserter(constAgents),
        [](const std::shared_ptr<Algorithm::Agent>& agent) {
            return std::const_pointer_cast<const Algorithm::Agent>(agent);
        }
    );

    return constAgents;
}


bool Algorithm::AgentManager::containsAgent(std::shared_ptr<const Agent> agent) const
{
    auto iterator = this->agents.find(agent);
    return iterator != this->agents.end() && *iterator == agent;
}