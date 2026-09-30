

#include "selection/truncationSelector.h"

std::vector<std::shared_ptr<const Individual>> Selection::TruncationSelector::select(
                const std::map<std::shared_ptr<const Individual>, double, SharedLess<Individual>>& fitnessMap,
                size_t nbSelected, RNG::RNG& rng) const
{
    std::vector<std::pair<std::shared_ptr<const Individual>, double>> individualFitnesses = getOrderedFitness(fitnessMap, false);

    // Standard (mu+lambda) replacement
    std::vector<std::shared_ptr<const Individual>> selected;
    for (size_t idx = 0; idx < individualFitnesses.size() && idx < nbSelected; ++idx) {
        selected.push_back(individualFitnesses[idx].first);
    }
    return selected;
}