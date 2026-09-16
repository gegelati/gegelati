
#include "reproduction/replicator.h"

std::set<std::shared_ptr<Individual>, SharedLess<Individual>> Reproduction::Replicator::reproduce(
            std::vector<std::shared_ptr<const Individual>> parents, size_t nbOffspring, RNG::RNG& rng) const
{
    std::set<std::shared_ptr<Individual>, SharedLess<Individual>> offspring;
    for(size_t idx = 0; idx < nbOffspring; idx++) {
        offspring.insert(parents.at(rng.uniformSample<size_t>(0, parents.size() - 1)).get()->cloneSharedPtr());
    }
    return offspring;
}