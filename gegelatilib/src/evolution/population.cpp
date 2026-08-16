

#include "evolution/population.h"

// Declaration of static GPNnode ID Counter in local here because it creates
// error in the .h file for MSVC compiler See:
// https://discourse.cmake.org/t/exporting-a-static-data-member-of-a-class-for-dll-using-msvc/5892
static size_t POPULATION_COUNTER_ID = 0;

size_t Evolution::Population::incrementeCounter()
{
    return POPULATION_COUNTER_ID++;
}

size_t Evolution::Population::getPopulationIDCounter()
{
    return POPULATION_COUNTER_ID;
}

void Evolution::Population::resetPopulationIDCounter()
{
    POPULATION_COUNTER_ID = 0;
}


size_t Evolution::Population::getPopulationID() const
{
    return this->populationID;
}

void Evolution::Population::setPopulationID(size_t newID)
{
    this->populationID = newID;

    // Update the ID counter if needed
    if (newID >= POPULATION_COUNTER_ID) {
        POPULATION_COUNTER_ID = newID + 1;
    }
}

bool Evolution::operator<(const Evolution::Population& a, const Evolution::Population& b)
{
    return a.getPopulationID() < b.getPopulationID();
}

bool Evolution::operator==(const Evolution::Population& a, const Evolution::Population& b)
{
    return a.getPopulationID() == b.getPopulationID();
}
bool Evolution::operator!=(const Evolution::Population& a, const Evolution::Population& b)
{
    return a.getPopulationID() != b.getPopulationID();
}


std::set<std::unique_ptr<Evolution::Individual>>::iterator Evolution::Population::getIndividualFromCst(const Individual& individual)
{
    auto iterator = this->individuals.find(&individual);
    if(iterator == this->individuals.end() || (*iterator).get() != &individual){
        throw std::runtime_error("Population::getIndividualFromCst: the given individual is not managed by this population.");
    }

    return iterator;
}

bool Evolution::Population::containsIndividual(const Individual& individual) const
{
    auto iterator = this->individuals.find(&individual);
    return iterator != this->individuals.end() && (*iterator).get() == &individual;
}

Evolution::Individual& Evolution::Population::getMutableIndividual(const Individual& individual)
{
    return *(this->getIndividualFromCst(individual)->get());
}

const std::vector<std::reference_wrapper<const Evolution::Individual>> Evolution::Population::getIndividuals() const
{
    std::vector<std::reference_wrapper<const Evolution::Individual>> refs;
    for (const auto& ptr : individuals) {
        refs.emplace_back(std::cref(*ptr));
    }
    return refs;
}


const Evolution::Individual& Evolution::Population::createIndividual()
{
    this->individuals.insert(std::make_unique<Individual>());
    return **this->individuals.rbegin();
}

const Evolution::Individual& Evolution::Population::copyIndividual(const Individual& individual)
{
    const Individual& newIndividual = this->createIndividual();
    auto it = this->getIndividualFromCst(newIndividual);

    const Genotype& genotype = individual.getGenotype();
    Genotype& newGenotype = it->get()->getMutableGenotype();

    for(const Node::NodeGroup& group: genotype.getNodeGroups()) {
        newGenotype.addNodeGroup();
        Node::NodeGroup& newNodeGroup = newGenotype.getMutableNodeGroup(newGenotype.getSize() - 1);
        
        for(const Node::GPNode& node: group.getNodes()) {
            newNodeGroup.addNode(std::make_unique<Node::GPNode>(node.getValues()));
        }
    }

    return newIndividual;
}

void Evolution::Population::emptyIndividual(const Individual& individual)
{
    auto it = this->getIndividualFromCst(individual);

    Genotype& genotype = it->get()->getMutableGenotype();
    while (genotype.getSize() > 0) {
        Node::NodeGroup& nodeGroup = genotype.getMutableNodeGroup(0);
        while(nodeGroup.getSize() > 0) {
            nodeGroup.removeNode(0);
        }
        genotype.removeNodeGroup(0);
    } 
}

void Evolution::Population::deleteIndividual(const Individual& individual)
{
    this->emptyIndividual(individual);
    this->individuals.erase(this->getIndividualFromCst(individual));
}

void Evolution::Population::clearIndividuals()
{
    while(this->individuals.size() > 0){
        this->deleteIndividual(**this->individuals.begin());
    }
}

size_t Evolution::Population::size() const
{
    return this->individuals.size();
}