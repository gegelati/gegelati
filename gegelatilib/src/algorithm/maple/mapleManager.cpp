
#include "algorithm/maple/mapleManager.h"


std::shared_ptr<Algorithm::Maple::MapleAgent> Algorithm::Maple::MapleManager::getMapleAgentFromCst(const Agent& agent)
{
    return std::dynamic_pointer_cast<MapleAgent>(this->getAgentFromCst(agent));
}

const std::vector<std::weak_ptr<const Algorithm::Agent>> Algorithm::Maple::MapleManager::getAgents() const
{
    // No filtering on root agents for Maple, as all agents are action agents
    return Algorithm::AgentManager::getAgents();
}



std::weak_ptr<const Algorithm::Agent> Algorithm::Maple::MapleManager::createAgent(std::shared_ptr<const EvoGraph::Vertex> vertex)
{
    std::shared_ptr<const EvoGraph::Team> team = std::dynamic_pointer_cast<const EvoGraph::Team>(vertex);
    if(team == nullptr){
        throw std::runtime_error("MapleManager::createAgent: trying to create an agent on an element from the graph that is not a team.");
    }

    this->agents.insert(std::make_shared<MapleAgent>(vertex, this->getAlgorithmName()));
    return *this->agents.rbegin();
}


void Algorithm::Maple::MapleManager::deleteAgent(const Agent& agent, std::shared_ptr<EvoGraph::Graph> graph)
{
    // Remove the action vertex from the graph
    graph->removeVertex(*this->getMapleAgentFromCst(agent)->getVertex());

    auto iterator = this->agents.find(&agent);
    this->agents.erase(iterator);   
}


std::unique_ptr<Algorithm::ExecutionEngine> Algorithm::Maple::MapleManager::createExecutionEngine(std::vector<std::reference_wrapper<const Data::DataHandler>> dataSources, bool isTraining) const
{
    auto engine = std::make_unique<Maple::MapleExecutionEngine>(this->outputs, this->algorithmName, isTraining);


    engine->setProgramExecutionEngine(std::move(this->cGetSubManager(this->programAlgorithmName)
                      ->createExecutionEngine(dataSources, isTraining))
    );

    return engine;
}