

#include "representation/agentManager.h"

std::set<std::unique_ptr<Representation::Agent>>::iterator Representation::AgentManager::getAgentFromCst(const Agent& agent)
{
    auto iterator = this->agents.find(&agent);
    if(iterator == this->agents.end() || (*iterator).get() != &agent){
        throw std::invalid_argument("AgentManager::getAgentFromCst: the given agent is not managed by this manager.");
    }

    return iterator;
}

bool Representation::AgentManager::containsAgent(const Agent& agent) const
{
    auto iterator = this->agents.find(&agent);
    return iterator != this->agents.end() && (*iterator).get() == &agent;
}

bool Representation::AgentManager::isAgentAccessible(const Agent& agent) const
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

void Representation::AgentManager::addSubManager(AgentManager& subManager)
{
    this->subManagers.insert({subManager.getRepresentationID(), subManager});
}

void Representation::AgentManager::addAggregatedManager(const AgentManager& managerAggregated)
{
    if(typeid(*this) != typeid(managerAggregated)){
        throw std::runtime_error("Representation::AgentManager::addRepresentationManagerAccess: Cannot add access to a manager of a different type.");
    }

    this->aggregatedManagers.push_back(managerAggregated);
}

const Representation::AgentManager& Representation::AgentManager::getAggregatedManager(uint64_t representationID) const{
    for(const auto& manager: this->aggregatedManagers){
        if(manager.get().getRepresentationID() == representationID){
            return manager;
        }
    }
    throw std::runtime_error("Representation::AgentManager::getAggregatedManager aggregated manager not found for the specific id");
}

Representation::AgentManager& Representation::AgentManager::getSubManager(uint64_t representationID){
    auto it = this->subManagers.find(representationID);
    if(it == this->subManagers.end()){
        throw std::runtime_error("Representation::AgentManager::getSubManager subManager not found for the specific id");
    }
    return it->second;
}

const Representation::AgentManager& Representation::AgentManager::cGetSubManager(uint64_t representationID) const{
    auto it = this->subManagers.find(representationID);
    if(it == this->subManagers.end()){
        throw std::runtime_error("Representation::AgentManager::getSubManager subManager not found for the specific id");
    }
    return it->second;
}

const std::vector<std::reference_wrapper<const Representation::AgentManager>>& Representation::AgentManager::getAggregatedManagers() const
{
    return this->aggregatedManagers;
}

const std::vector<std::reference_wrapper<const Representation::Agent>> Representation::AgentManager::getAgents() const
{
    std::vector<std::reference_wrapper<const Representation::Agent>> refs;
    for (const auto& ptr : agents) {
        refs.emplace_back(std::cref(*ptr));
    }
    return refs;
}

void Representation::AgentManager::deleteAgent(const Agent& agent, EvoGraph::Graph& graph)
{
    this->emptyAgent(agent, graph);
    this->agents.erase(this->getAgentFromCst(agent));
}

void Representation::AgentManager::clearAgents(EvoGraph::Graph& graph)
{
    while(this->agents.size() > 0){
        this->deleteAgent(**this->agents.begin(), graph);
    }

    // Also clear sub managers;
    for(const auto& pair : this->subManagers) {
        pair.second.get().clearAgents(graph);
    }
}


void Representation::AgentManager::setNewAgentID(const Agent& agent, uint64_t newID)
{
    // Check that the agent to modify exists in the manager
    auto itAgent = this->getAgentFromCst(agent);

    // Check that no other vertex has the same ID
    for (const auto& vptr : this->agents) {
        if (vptr.get() != itAgent->get() && vptr->getAgentID() == newID) {
            throw std::runtime_error("Another agent with the same ID already "
                                     "exists in the manager.");
        }
    }

    // Modify the ID, but removed and add again the agent for that.
    auto tmp = this->agents.extract(itAgent);
    tmp.value()->setAgentID(newID);
    this->agents.insert(std::move(tmp));
}