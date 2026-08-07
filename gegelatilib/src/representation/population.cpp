

#include "representation/population.h"



std::set<std::unique_ptr<Representation::Individual>>::iterator Representation::Population::getIndividualFromCst(const Individual& individual)
{
    auto iterator = this->individuals.find(&individual);
    if(iterator == this->individuals.end() || (*iterator).get() != &individual){
        throw std::invalid_argument("Population::getIndividualFromCst: the given individual is not managed by this population.");
    }

    return iterator;
}

bool Representation::Population::containsIndividual(const Individual& individual) const
{
    auto iterator = this->individuals.find(&individual);
    return iterator != this->individuals.end() && (*iterator).get() == &individual;
}

bool Representation::Population::isIndividualAccessible(const Individual& individual) const
{
    if(this->containsIndividual(individual)){
        return true;
    }

    for(const auto& population: this->aggregatedPopulations){
        if(population.get().containsIndividual(individual)){
            return true;
        }
    }

    return false;
}

void Representation::Population::addSubPopulation(Population& subPopulation)
{
    this->subPopulations.insert({subPopulation.getRepresentationID(), subPopulation});
}

void Representation::Population::addAggregatedPopulation(const Population& populationAggregated)
{
    if(typeid(*this) != typeid(populationAggregated)){
        throw std::runtime_error("Representation::Population::addRepresentationPopulationAccess: Cannot add access to a population of a different type.");
    }

    this->aggregatedPopulations.push_back(populationAggregated);
}

const Representation::Population& Representation::Population::getAggregatedPopulation(uint64_t representationID) const{
    for(const auto& population: this->aggregatedPopulations){
        if(population.get().getRepresentationID() == representationID){
            return population;
        }
    }
    throw std::runtime_error("Representation::Population::getAggregatedPopulation aggregated population not found for the specific id");
}

Representation::Population& Representation::Population::getSubPopulation(uint64_t representationID){
    auto it = this->subPopulations.find(representationID);
    if(it == this->subPopulations.end()){
        throw std::runtime_error("Representation::Population::getSubPopulation subPopulation not found for the specific id");
    }
    return it->second;
}

const Representation::Population& Representation::Population::cGetSubPopulation(uint64_t representationID) const{
    auto it = this->subPopulations.find(representationID);
    if(it == this->subPopulations.end()){
        throw std::runtime_error("Representation::Population::getSubPopulation subPopulation not found for the specific id");
    }
    return it->second;
}

const std::vector<std::reference_wrapper<const Representation::Population>>& Representation::Population::getAggregatedPopulations() const
{
    return this->aggregatedPopulations;
}

const std::vector<std::reference_wrapper<const Representation::Individual>> Representation::Population::getIndividuals() const
{
    std::vector<std::reference_wrapper<const Representation::Individual>> refs;
    for (const auto& ptr : individuals) {
        refs.emplace_back(std::cref(*ptr));
    }
    return refs;
}

void Representation::Population::deleteIndividual(const Individual& individual, EvoGraph::Graph& graph)
{
    this->emptyIndividual(individual, graph);
    this->individuals.erase(this->getIndividualFromCst(individual));
}

void Representation::Population::clearIndividuals(EvoGraph::Graph& graph)
{
    while(this->individuals.size() > 0){
        this->deleteIndividual(**this->individuals.begin(), graph);
    }

    // Also clear sub populations;
    for(const auto& pair : this->subPopulations) {
        pair.second.get().clearIndividuals(graph);
    }
}


void Representation::Population::setNewIndividualID(const Individual& individual, uint64_t newID)
{
    // Check that the individual to modify exists in the population
    auto itIndividual = this->getIndividualFromCst(individual);

    // Check that no other vertex has the same ID
    for (const auto& vptr : this->individuals) {
        if (vptr.get() != itIndividual->get() && vptr->getIndividualID() == newID) {
            throw std::runtime_error("Another individual with the same ID already "
                                     "exists in the population.");
        }
    }

    // Modify the ID, but removed and add again the individual for that.
    auto tmp = this->individuals.extract(itIndividual);
    tmp.value()->setIndividualID(newID);
    this->individuals.insert(std::move(tmp));
}