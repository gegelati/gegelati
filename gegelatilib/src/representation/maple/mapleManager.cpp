
#include "representation/maple/mapleManager.h"


Representation::Maple::MapleAgent& Representation::Maple::MapleManager::getMapleAgentFromCst(const Agent& agent)
{
    return dynamic_cast<MapleAgent&>(**this->getAgentFromCst(agent));
}

const std::vector<std::reference_wrapper<const Representation::Agent>> Representation::Maple::MapleManager::getAgents() const
{
    // No filtering on root agents for Maple, as all agents are action agents
    return Representation::AgentManager::getAgents();
}



const Representation::Agent& Representation::Maple::MapleManager::createAgent(std::optional<std::reference_wrapper<const EvoGraph::Vertex>> vertex)
{
    this->agents.insert(std::make_unique<MapleAgent>(vertex, this->getRepresentationID()));
    return **this->agents.rbegin();
}

std::unique_ptr<Representation::ExecutionEngine> Representation::Maple::MapleManager::createExecutionEngine(std::vector<std::reference_wrapper<const Data::DataHandler>> dataSources, bool isTraining) const
{
    auto engine = std::make_unique<Maple::MapleExecutionEngine>(this->outputs, this->representationID, isTraining);


    engine->setProgramExecutionEngine(std::move(this->cGetSubManager(this->programRepresentationID)
                      .createExecutionEngine(dataSources, isTraining))
    );

    return engine;
}