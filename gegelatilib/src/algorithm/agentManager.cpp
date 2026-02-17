

#include "algorithm/agentManager.h"

std::shared_ptr<Algorithm::Agent> Algorithm::AgentManager::getAgentFromCst(const Agent& agent)
{
    auto iterator = this->agents.find(&agent);
    if(iterator == this->agents.end() || (*iterator).get() != &agent){
        throw std::invalid_argument("AgentManager::getAgentFromCst: the given agent is not managed by this manager.");
    }

    return *iterator;
}

bool Algorithm::AgentManager::containsAgent(const Agent& agent) const
{
    auto iterator = this->agents.find(&agent);
    return iterator != this->agents.end() && (*iterator).get() == &agent;
}

bool Algorithm::AgentManager::isAgentAccessible(const Agent& agent) const
{
    if(this->containsAgent(agent)){
        return true;
    }

    for(const auto& manager: this->aggregatedManagers){
        if(manager.get().containsAgent(agent)){
            return true;
        }
    }

    return false;
}

void Algorithm::AgentManager::addSubManager(std::shared_ptr<AgentManager> subManager)
{
    this->subManagers.insert({subManager->getAlgorithmName(), subManager});
}

void Algorithm::AgentManager::addAggregatedManager(const AgentManager& managerAggregated)
{
    if(typeid(*this) != typeid(managerAggregated)){
        throw std::runtime_error("Algorithm::AgentManager::addAlgorithmManagerAccess: Cannot add access to a manager of a different type.");
    }

    this->aggregatedManagers.push_back(managerAggregated);
}

const Algorithm::AgentManager& Algorithm::AgentManager::getAggregatedManager(std::string nameAlgorithm) const{
    for(const auto& manager: this->aggregatedManagers){
        if(manager.get().getAlgorithmName() == nameAlgorithm){
            return manager;
        }
    }
    throw std::runtime_error("Algorithm::AgentManager::getAggregatedManager aggregated manager not found for the specific name");
}

std::shared_ptr<Algorithm::AgentManager> Algorithm::AgentManager::getSubManager(std::string nameAlgorithm){
    auto it = this->subManagers.find(nameAlgorithm);
    if(it == this->subManagers.end()){
        throw std::runtime_error("Algorithm::AgentManager::getSubManager subManager not found for the specific name");
    }
    return it->second;
}

std::shared_ptr<const Algorithm::AgentManager> Algorithm::AgentManager::cGetSubManager(std::string nameAlgorithm) const{
    auto it = this->subManagers.find(nameAlgorithm);
    if(it == this->subManagers.end()){
        throw std::runtime_error("Algorithm::AgentManager::getSubManager subManager not found for the specific name");
    }
    return it->second;
}

const std::vector<std::reference_wrapper<const Algorithm::AgentManager>>& Algorithm::AgentManager::getAggregatedManagers() const
{
    return this->aggregatedManagers;
}

const std::vector<std::reference_wrapper<const Algorithm::Agent>> Algorithm::AgentManager::getAgents() const
{
    std::vector<std::reference_wrapper<const Algorithm::Agent>> refs;
    for (const auto& ptr : agents) {
        refs.emplace_back(std::cref(*ptr));
    }
    return refs;
}

void Algorithm::AgentManager::deleteAgent(const Agent& agent, std::shared_ptr<EvoGraph::Graph> graph)
{
    this->emptyAgent(agent, graph);
    this->agents.erase(this->getAgentFromCst(agent));
}

void Algorithm::AgentManager::clearAgents(std::shared_ptr<EvoGraph::Graph> graph)
{
    while(this->agents.size() > 0){
        this->deleteAgent(**this->agents.begin(), graph);
    }
}