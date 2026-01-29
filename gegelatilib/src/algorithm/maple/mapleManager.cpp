
#include "algorithm/maple/mapleManager.h"


std::shared_ptr<Algorithm::Maple::MapleAgent> Algorithm::Maple::MapleManager::getMapleAgentFromCst(std::shared_ptr<const Agent> agent)
{
    auto iterator = this->agents.find(agent);
    if(iterator == this->agents.end() || *iterator != agent){
        throw std::invalid_argument("MapleManager::getMapleAgentFromCst: the given agent is not managed by this manager.");
    }

    return std::dynamic_pointer_cast<MapleAgent>(*iterator);
}

const std::vector<std::shared_ptr<const Algorithm::Agent>> Algorithm::Maple::MapleManager::getAgents() const
{
    // No filtering on root agents for Maple, as all agents are action agents
    return Algorithm::AgentManager::getAgents();
}



std::shared_ptr<const Algorithm::Agent> Algorithm::Maple::MapleManager::createAgent(std::shared_ptr<const EvoGraph::Vertex> vertex)
{
    if(!std::dynamic_pointer_cast<const EvoGraph::Team>(vertex)){
        throw std::runtime_error("MapleManager::createAgent: MapleAgent can only be created with team vertex.");
    }
    this->agents.insert(std::make_shared<MapleAgent>(vertex, this->getAlgorithmName()));
    return *this->agents.rbegin();
}


void Algorithm::Maple::MapleManager::deleteAgent(std::shared_ptr<const Agent> agent, std::shared_ptr<EvoGraph::Graph> graph)
{
    // Remove the action vertex from the graph
    graph->removeVertex(*this->getMapleAgentFromCst(agent)->getVertex());

    auto iterator = this->agents.find(agent);
    this->agents.erase(iterator);   
}


std::unique_ptr<Algorithm::ExecutionEngine> Algorithm::Maple::MapleManager::createExecutionEngine(std::vector<std::reference_wrapper<const Data::DataHandler>> dataSources, bool isTraining) const
{
    auto engine = std::make_unique<Maple::MapleExecutionEngine>(this->outputs, this->algorithmName, this->archive, isTraining);


    engine->setProgramExecutionEngine(std::move(this->cGetSubManager(this->programAlgorithmName)
                      ->createExecutionEngine(dataSources, isTraining))
    );

    return engine;
}