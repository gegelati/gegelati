

#include "selectors/truncation.h"

std::vector<std::shared_ptr<const Evolution::Individual>> Selectors::Truncation::select(
                const std::set<std::shared_ptr<const Evolution::Individual>, SharedLess<Evolution::Individual>>& individuals,
                size_t nbSelected, RNG::RNG& rng) const
{
    std::vector<std::pair<double, std::shared_ptr<const Evolution::Individual>>> ranked = this->getRankedScores(individuals);

    // Standard (mu+lambda) replacement
    std::vector<std::shared_ptr<const Evolution::Individual>> selected;
    for (size_t idx = 0; idx < ranked.size() && idx < nbSelected; ++idx) {
        selected.push_back(ranked[idx].second);
    }
    return selected;
}