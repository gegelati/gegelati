
#include "evolution/reproduction.h"

std::set<std::shared_ptr<Evolution::Individual>, SharedLess<Evolution::Individual>> Evolution::Reproduction::reproduce(
            std::vector<std::shared_ptr<const Evolution::Individual>> parents, size_t nbOffspring, RNG::RNG& rng) const
{
    std::set<std::shared_ptr<Evolution::Individual>, SharedLess<Evolution::Individual>> offspring;
    for(size_t idx = 0; idx < nbOffspring; idx++) {
        offspring.insert(parents.at(rng.uniformSample<size_t>(0, parents.size() - 1)).get()->cloneSharedPtr());
    }
    return offspring;
}