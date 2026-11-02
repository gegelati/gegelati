

#include "selection/truncationSelector.h"

std::vector<uint64_t> Selection::TruncationSelector::select(
                const std::map<uint64_t, double>& fitnessMap,
                size_t nbSelected, RNG::RNG& rng) const
{
    std::vector<std::pair<uint64_t, double>> individualFitnesses = getOrderedFitness(fitnessMap, false);

    // Standard (mu+lambda) replacement
    std::vector<uint64_t> selected;
    for (size_t idx = 0; idx < individualFitnesses.size() && idx < nbSelected; ++idx) {
        selected.push_back(individualFitnesses[idx].first);
    }
    return selected;
}