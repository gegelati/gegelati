#include "util/individualUtils.h"

#include <cmath>

std::shared_ptr<const Individual> Util::bestIndividual(const std::vector<std::shared_ptr<const Individual>>& individuals, const std::map<uint64_t, double>& fitness)
{
    std::shared_ptr<const Individual> best = nullptr;
    double bestFitness = 0.0;
    for(const std::shared_ptr<const Individual>& individual: individuals) {
        const auto found = fitness.find(individual->getIndividualID());
        if(found == fitness.end() || std::isnan(found->second)) {
            continue;
        }
        if(best == nullptr || found->second > bestFitness || (found->second == bestFitness && individual->getIndividualID() < best->getIndividualID())) {
            best = individual;
            bestFitness = found->second;
        }
    }
    return best;
}

std::unordered_map<uint64_t, std::shared_ptr<const Individual>> Util::makeIndex(const std::vector<std::shared_ptr<const Individual>>& individuals)
{
    std::unordered_map<uint64_t, std::shared_ptr<const Individual>> index;
    index.reserve(individuals.size());
    for(const std::shared_ptr<const Individual>& individual: individuals) {
        index.emplace(individual->getIndividualID(), individual);
    }
    return index;
}
