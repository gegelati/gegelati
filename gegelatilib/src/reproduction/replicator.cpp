
#include "reproduction/replicator.h"

std::vector<std::shared_ptr<Individual>> Reproduction::Replicator::reproduce(
            std::vector<std::shared_ptr<const Individual>> parents, size_t nbOffspring, RNG::RNG& rng) const
{
    if(parents.empty() && nbOffspring > 0) {
        throw std::runtime_error("Reproduction::Replicator::reproduce: cannot reproduce without parents");
    }

    std::vector<std::shared_ptr<Individual>> offspring;
    offspring.reserve(nbOffspring);
    for(size_t idx = 0; idx < nbOffspring; idx++) {
        offspring.push_back(parents.at(rng.uniformSample<size_t>(0, parents.size() - 1)).get()->cloneSharedPtr());
    }
    return offspring;
}