

#include "selection/truncationSelector.h"

std::vector<std::shared_ptr<const Individual>> Selection::TruncationSelector::select(
                const std::set<std::shared_ptr<const Individual>, SharedLess<Individual>>& individuals,
                size_t nbSelected, RNG::RNG& rng) const
{
    std::vector<std::pair<double, std::shared_ptr<const Individual>>> ranked = this->assignFitness(individuals);

    // Standard (mu+lambda) replacement
    std::vector<std::shared_ptr<const Individual>> selected;
    for (size_t idx = 0; idx < ranked.size() && idx < nbSelected; ++idx) {
        selected.push_back(ranked[idx].second);
    }
    return selected;
}