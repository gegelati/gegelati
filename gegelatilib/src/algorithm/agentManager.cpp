

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

std::shared_ptr<Algorithm::AgentManager> Algorithm::AgentManager::cGetSubManager(std::string nameAlgorithm) const{
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

void Algorithm::AgentManager::setElement(std::shared_ptr<const Agent> agent, std::shared_ptr<EvoGraph::Graph> graph, std::shared_ptr<const EvoGraph::Element> element)
{
    if(!graph->hasElement(*element)){
        throw std::runtime_error("AgentManager::setElement: trying to set the element to an agent, but the element is not in the graph.");
    }

    if(!this->containsAgent(agent)){
        throw std::runtime_error("AgentManager::setElement: Trying to set the element to an agent not present in the algorithm.");
    }

    // Set the element
    (*this->agents.find(agent))->setElement(element);
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