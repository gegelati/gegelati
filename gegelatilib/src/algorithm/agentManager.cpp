

#include "algorithm/agentManager.h"




void Algorithm::AgentManager::addSubManager(std::shared_ptr<AgentManager> subManager)
{
    this->subManagers.insert({subManager->getAlgorithmName(), subManager});
}

std::shared_ptr<Algorithm::AgentManager> Algorithm::AgentManager::getSubManager(std::string nameAlgorithm){
    auto it = this->subManagers.find(nameAlgorithm);
    if(it == this->subManagers.end()){
        throw std::runtime_error("Algorithm::AgentManager::getSubManager subManager not found for the specific name");
    }
    return it->second;
}

const std::vector<std::shared_ptr<const Algorithm::Agent>> Algorithm::AgentManager::getAgents() const
{
    return std::vector<std::shared_ptr<const Algorithm::Agent>>(
        this->agents.begin(),
        this->agents.end()
    );
}


bool Algorithm::AgentManager::containsAgent(std::shared_ptr<const Agent> agent) const
{
    auto iterator = this->agents.find(agent);
    return iterator != this->agents.end() && *iterator == agent;
}

void Algorithm::AgentManager::clearAgents()
{
    for(auto agent: this->agents){
        this->deleteAgent(agent, nullptr);
    }
}