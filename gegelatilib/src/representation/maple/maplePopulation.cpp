
#include "representation/maple/maplePopulation.h"


Representation::Maple::MapleIndividual& Representation::Maple::MaplePopulation::getMapleIndividualFromCst(const Individual& agent)
{
    return dynamic_cast<MapleIndividual&>(**this->getAgentFromCst(agent));
}

const std::vector<std::reference_wrapper<const Representation::Individual>> Representation::Maple::MaplePopulation::getAgents() const
{
    // No filtering on root agents for Maple, as all agents are action agents
    return Representation::Population::getAgents();
}



const Representation::Individual& Representation::Maple::MaplePopulation::createAgent(std::optional<std::reference_wrapper<const EvoGraph::Vertex>> vertex)
{
    this->agents.insert(std::make_unique<MapleIndividual>(vertex, this->getRepresentationID()));
    return **this->agents.rbegin();
}

std::unique_ptr<Representation::ExecutionEngine> Representation::Maple::MaplePopulation::createExecutionEngine(std::vector<std::reference_wrapper<const Data::DataHandler>> dataSources, bool isTraining) const
{
    auto engine = std::make_unique<Maple::MapleExecutionEngine>(this->outputs, this->representationID, isTraining);


    engine->setProgramExecutionEngine(std::move(this->cGetSubPopulation(this->programRepresentationID)
                      .createExecutionEngine(dataSources, isTraining))
    );

    return engine;
}