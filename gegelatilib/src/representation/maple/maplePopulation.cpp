
#include "representation/maple/maplePopulation.h"


Representation::Maple::MapleIndividual& Representation::Maple::MaplePopulation::getMapleIndividualFromCst(const Individual& individual)
{
    return dynamic_cast<MapleIndividual&>(**this->getIndividualFromCst(individual));
}

const std::vector<std::reference_wrapper<const Representation::Individual>> Representation::Maple::MaplePopulation::getIndividuals() const
{
    // No filtering on root individuals for Maple, as all individuals are action individuals
    return Representation::Population::getIndividuals();
}



const Representation::Individual& Representation::Maple::MaplePopulation::createIndividual(std::optional<std::reference_wrapper<const EvoGraph::Vertex>> vertex)
{
    this->individuals.insert(std::make_unique<MapleIndividual>(vertex, this->getRepresentationID()));
    return **this->individuals.rbegin();
}

std::unique_ptr<Representation::ExecutionEngine> Representation::Maple::MaplePopulation::createExecutionEngine(std::vector<std::reference_wrapper<const Data::DataHandler>> dataSources, bool isTraining) const
{
    auto engine = std::make_unique<Maple::MapleExecutionEngine>(this->outputs, this->representationID, isTraining);


    engine->setProgramExecutionEngine(std::move(this->cGetSubPopulation(this->programRepresentationID)
                      .createExecutionEngine(dataSources, isTraining))
    );

    return engine;
}