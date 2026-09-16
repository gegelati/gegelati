

#include "selection/random.h"

std::vector<std::shared_ptr<const Evolution::Individual>> Selectors::Random::select(
                const std::set<std::shared_ptr<const Evolution::Individual>, SharedLess<Evolution::Individual>>& individuals,
                size_t nbSelected, RNG::RNG& rng) const
{
    std::vector<std::pair<double, std::shared_ptr<const Evolution::Individual>>> ranked = this->getRankedScores(individuals);

    // Standard (mu+lambda) replacement
    std::vector<std::shared_ptr<const Evolution::Individual>> selected;
    while (ranked.size() > 0 && selected.size() < nbSelected) {
        size_t sampledIndex = rng.uniformSample<size_t>(0, ranked.size() - 1);
        selected.push_back(ranked[sampledIndex].second);

        // Remove selected individual if replacement is not allowed.
        if(!this->replacement) {
            ranked.erase(ranked.begin() + sampledIndex);
        }
    }
    return selected;
}