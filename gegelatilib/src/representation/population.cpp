

#include "representation/population.h"

std::set<std::unique_ptr<Representation::Individual>>::iterator Representation::Population::getAgentFromCst(const Individual& agent)
{
    auto iterator = this->agents.find(&agent);
    if(iterator == this->agents.end() || (*iterator).get() != &agent){
        throw std::invalid_argument("Population::getAgentFromCst: the given agent is not managed by this population.");
    }

    return iterator;
}

bool Representation::Population::containsAgent(const Individual& agent) const
{
    auto iterator = this->agents.find(&agent);
    return iterator != this->agents.end() && (*iterator).get() == &agent;
}

bool Representation::Population::isAgentAccessible(const Individual& agent) const
{
    if(this->containsAgent(agent)){
        return true;
    }

    for(const auto& population: this->aggregatedPopulations){
        if(population.get().containsAgent(agent)){
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

const std::vector<std::reference_wrapper<const Representation::Individual>> Representation::Population::getAgents() const
{
    std::vector<std::reference_wrapper<const Representation::Individual>> refs;
    for (const auto& ptr : agents) {
        refs.emplace_back(std::cref(*ptr));
    }
    return refs;
}

void Representation::Population::deleteAgent(const Individual& agent, EvoGraph::Graph& graph)
{
    this->emptyAgent(agent, graph);
    this->agents.erase(this->getAgentFromCst(agent));
}

void Representation::Population::clearAgents(EvoGraph::Graph& graph)
{
    while(this->agents.size() > 0){
        this->deleteAgent(**this->agents.begin(), graph);
    }

    // Also clear sub populations;
    for(const auto& pair : this->subPopulations) {
        pair.second.get().clearAgents(graph);
    }
}


void Representation::Population::setNewAgentID(const Individual& agent, uint64_t newID)
{
    // Check that the agent to modify exists in the population
    auto itAgent = this->getAgentFromCst(agent);

    // Check that no other vertex has the same ID
    for (const auto& vptr : this->agents) {
        if (vptr.get() != itAgent->get() && vptr->getAgentID() == newID) {
            throw std::runtime_error("Another agent with the same ID already "
                                     "exists in the population.");
        }
    }

    // Modify the ID, but removed and add again the agent for that.
    auto tmp = this->agents.extract(itAgent);
    tmp.value()->setAgentID(newID);
    this->agents.insert(std::move(tmp));
}