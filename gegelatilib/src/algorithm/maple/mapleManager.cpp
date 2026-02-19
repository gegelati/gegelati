
#include "algorithm/maple/mapleManager.h"


Algorithm::Maple::MapleAgent& Algorithm::Maple::MapleManager::getMapleAgentFromCst(const Agent& agent)
{
    return dynamic_cast<MapleAgent&>(**this->getAgentFromCst(agent));
}

const std::vector<std::reference_wrapper<const Algorithm::Agent>> Algorithm::Maple::MapleManager::getAgents() const
{
    // No filtering on root agents for Maple, as all agents are action agents
    return Algorithm::AgentManager::getAgents();
}



const Algorithm::Agent& Algorithm::Maple::MapleManager::createAgent(std::optional<std::reference_wrapper<const EvoGraph::Vertex>> vertex)
{
    this->agents.insert(std::make_unique<MapleAgent>(vertex, this->getAlgorithmID()));
    return **this->agents.rbegin();
}

std::unique_ptr<Algorithm::ExecutionEngine> Algorithm::Maple::MapleManager::createExecutionEngine(std::vector<std::reference_wrapper<const Data::DataHandler>> dataSources, bool isTraining) const
{
    auto engine = std::make_unique<Maple::MapleExecutionEngine>(this->outputs, this->algorithmID, isTraining);


    engine->setProgramExecutionEngine(std::move(this->cGetSubManager(this->programAlgorithmID)
                      .createExecutionEngine(dataSources, isTraining))
    );

    return engine;
}