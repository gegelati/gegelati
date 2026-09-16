

#include "population.h"

// Declaration of static GPNnode ID Counter in local here because it creates
// error in the .h file for MSVC compiler See:
// https://discourse.cmake.org/t/exporting-a-static-data-member-of-a-class-for-dll-using-msvc/5892
static size_t POPULATION_COUNTER_ID = 0;

size_t Population::incrementeCounter()
{
    return POPULATION_COUNTER_ID++;
}

size_t Population::getPopulationIDCounter()
{
    return POPULATION_COUNTER_ID;
}

void Population::resetPopulationIDCounter()
{
    POPULATION_COUNTER_ID = 0;
}


size_t Population::getPopulationID() const
{
    return this->populationID;
}

void Population::setPopulationID(size_t newID)
{
    this->populationID = newID;

    // Update the ID counter if needed
    if (newID >= POPULATION_COUNTER_ID) {
        POPULATION_COUNTER_ID = newID + 1;
    }
}

bool operator<(const Population& a, const Population& b)
{
    return a.getPopulationID() < b.getPopulationID();
}

bool operator==(const Population& a, const Population& b)
{
    return a.getPopulationID() == b.getPopulationID();
}
bool operator!=(const Population& a, const Population& b)
{
    return a.getPopulationID() != b.getPopulationID();
}


std::set<std::shared_ptr<Individual>>::iterator Population::getIndividualFromCst(const Individual& individual)
{
    auto iterator = this->individuals.find(&individual);
    if(iterator == this->individuals.end() || (*iterator).get() != &individual){
        throw std::runtime_error("Population::getIndividualFromCst: the given individual is not managed by this population.");
    }

    return iterator;
}

bool Population::containsIndividual(const Individual& individual) const
{
    auto iterator = this->individuals.find(&individual);
    return iterator != this->individuals.end() && (*iterator).get() == &individual;
}

Individual& Population::getMutableIndividual(const Individual& individual)
{
    return *(this->getIndividualFromCst(individual)->get());
}

std::set<std::reference_wrapper<const Individual>> Population::getIndividuals() const
{
    std::set<std::reference_wrapper<const Individual>> vect;
    for (const auto& ptr : individuals) {
        vect.insert(*ptr);
    }
    return vect;
}

std::set<std::reference_wrapper<const Individual>> Population::getProtectedIndividuals() const
{
    std::set<std::reference_wrapper<const Individual>> vect;
    for (const auto& ptr : individuals) {
        if(ptr.use_count() > 1) {
            vect.insert(*ptr);
        }
    }
    return vect;
}

std::set<std::reference_wrapper<const Individual>> Population::getNotProtectedIndividuals() const
{
    std::set<std::reference_wrapper<const Individual>> vect;
    for (const auto& ptr : individuals) {
        if(ptr.use_count() == 1) {
            vect.insert(*ptr);
        }
    }
    return vect;
}

std::vector<std::weak_ptr<const Individual>> Population::getIndividualPtrs() const
{
    std::vector<std::weak_ptr<const Individual>> vect;
    for (const auto& ptr : individuals) {
        vect.push_back(ptr);
    }
    return vect;
}

const Individual& Population::addIndividual(std::unique_ptr<Individual> individual)
{
    this->individuals.insert(std::move(individual));
    return **this->individuals.rbegin();
}

void Population::emptyIndividual(const Individual& individual)
{
    auto it = this->getIndividualFromCst(individual);

}

bool Population::deleteIndividual(const Individual& individual)
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

void Population::clearIndividuals() {

    // Keep trying until the container is empty
    while (this->individuals.size() > 0) {
        std::size_t const sizeBefore = this->individuals.size();

        // One full removing iteration: attempt to delete from the front until empty
        auto individualsCopy(this->getIndividuals());
        for(const Individual& individual: individualsCopy){
            this->deleteIndividual(individual);
        }

        if (this->individuals.size() == sizeBefore) {
            throw std::runtime_error("Population::clearIndividuals: individuals remain after a full removal pass. Check deleteIndividual ordering or container management. However, this might be intended, this error could be removed in future versions");
        }
        
    }
}

size_t Population::size() const
{
    return this->individuals.size();
}

size_t Population::sizeProtected() const
{
    return std::count_if(this->individuals.begin(), this->individuals.end(),
                         [](const std::shared_ptr<Individual>& individual) {
                             return individual.use_count() > 1;
                         });
}

size_t Population::sizeNotProtected() const
{
    return std::count_if(this->individuals.begin(), this->individuals.end(),
                         [](const std::shared_ptr<Individual>& individual) {
                             return individual.use_count() == 1;
                         });
}
