

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


std::set<std::shared_ptr<Evolution::Individual>>::iterator Evolution::Population::getIndividualFromCst(const Individual& individual)
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

std::set<std::reference_wrapper<const Evolution::Individual>> Evolution::Population::getIndividuals() const
{
    std::set<std::reference_wrapper<const Evolution::Individual>> vect;
    for (const auto& ptr : individuals) {
        vect.insert(*ptr);
    }
    return vect;
}

std::set<std::reference_wrapper<const Evolution::Individual>> Evolution::Population::getProtectedIndividuals() const
{
    std::set<std::reference_wrapper<const Evolution::Individual>> vect;
    for (const auto& ptr : individuals) {
        if(ptr.use_count() > 1) {
            vect.insert(*ptr);
        }
    }
    return vect;
}

std::set<std::reference_wrapper<const Evolution::Individual>> Evolution::Population::getNotProtectedIndividuals() const
{
    std::set<std::reference_wrapper<const Evolution::Individual>> vect;
    for (const auto& ptr : individuals) {
        if(ptr.use_count() == 1) {
            vect.insert(*ptr);
        }
    }
    return vect;
}

std::vector<std::weak_ptr<const Evolution::Individual>> Evolution::Population::getIndividualPtrs() const
{
    std::vector<std::weak_ptr<const Evolution::Individual>> vect;
    for (const auto& ptr : individuals) {
        vect.push_back(ptr);
    }
    return vect;
}

const Evolution::Individual& Evolution::Population::addIndividual(std::unique_ptr<Individual> individual)
{
    this->individuals.insert(std::move(individual));
    return **this->individuals.rbegin();
}

const Evolution::Individual& Evolution::Population::addIndividual()
{
    return this->addIndividual(std::make_unique<Individual>());
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

bool Evolution::Population::deleteIndividual(const Individual& individual)
{
    auto it = this->getIndividualFromCst(individual);
    if(it->use_count() > 1) {
        // Individual is aggregated
        return false;
    }
    this->emptyIndividual(individual);
    //this->individualAggregations.erase(individual);
    this->individuals.erase(this->getIndividualFromCst(individual));
    return true;
}

void Evolution::Population::clearIndividuals() {

    // Keep trying until the container is empty
    while (this->individuals.size() > 0) {
        std::size_t const sizeBefore = this->individuals.size();

        // One full removing iteration: attempt to delete from the front until empty
        auto individualsCopy(this->getIndividuals());
        for(const Individual& individual: individualsCopy){
            this->deleteIndividual(individual);
        }

        if (this->individuals.size() == sizeBefore) {
            throw std::runtime_error("Evolution::Population::clearIndividuals: individuals remain after a full removal pass. Check deleteIndividual ordering or container management. However, this might be intended, this error could be removed in future versions");
        }
        
    }
}

size_t Evolution::Population::size() const
{
    return this->individuals.size();
}

size_t Evolution::Population::sizeProtected() const
{
    return std::count_if(this->individuals.begin(), this->individuals.end(),
                         [](const std::shared_ptr<Individual>& individual) {
                             return individual.use_count() > 1;
                         });
}

size_t Evolution::Population::sizeNotProtected() const
{
    return std::count_if(this->individuals.begin(), this->individuals.end(),
                         [](const std::shared_ptr<Individual>& individual) {
                             return individual.use_count() == 1;
                         });
}
