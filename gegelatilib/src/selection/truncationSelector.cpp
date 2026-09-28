

#include "selection/truncationSelector.h"

std::vector<std::shared_ptr<const Individual>> Selection::TruncationSelector::select(
                const std::vector<std::pair<double, std::shared_ptr<const Individual>>>& individualFitnesses,
                size_t nbSelected, RNG::RNG& rng) const
{

    // Standard (mu+lambda) replacement
    std::vector<std::shared_ptr<const Individual>> selected;
    for (size_t idx = 0; idx < individualFitnesses.size() && idx < nbSelected; ++idx) {
        selected.push_back(individualFitnesses[idx].second);
    }
    return selected;
}