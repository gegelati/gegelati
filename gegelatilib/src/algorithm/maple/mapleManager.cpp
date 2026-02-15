
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
    this->agents.insert(std::make_shared<MapleAgent>(vertex, this->getAlgorithmName()));
    return *this->agents.rbegin();
}

std::unique_ptr<Algorithm::ExecutionEngine> Algorithm::Maple::MapleManager::createExecutionEngine(std::vector<std::reference_wrapper<const Data::DataHandler>> dataSources, bool isTraining) const
{
    auto engine = std::make_unique<Maple::MapleExecutionEngine>(this->outputs, this->algorithmName, isTraining);


    engine->setProgramExecutionEngine(std::move(this->cGetSubManager(this->programAlgorithmName)
                      ->createExecutionEngine(dataSources, isTraining))
    );

    return engine;
}