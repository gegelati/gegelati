#include "evolution/population.h"

#include <stdexcept>

Evolution::Population::Population(std::shared_ptr<const Representations::Representation> representation, size_t size, std::string name)
    : populationName{std::move(name)}, representation{std::move(representation)}, size{size}
{
    if(this->representation == nullptr) {
        throw std::runtime_error("Evolution::Population: the representation cannot be null");
    }
    if(this->populationName.empty()) {
        throw std::runtime_error("Evolution::Population: the name cannot be empty");
    }
}

std::string Evolution::Population::individuals() const
{
    return this->populationName;
}

std::string Evolution::Population::data(const std::string& name) const
{
    return this->populationName + "." + name;
}
