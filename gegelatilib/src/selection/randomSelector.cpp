

#include "selection/randomSelector.h"

std::vector<std::shared_ptr<const Individual>> Selection::RandomSelector::select(
                const std::vector<std::pair<double, std::shared_ptr<const Individual>>>& individualFitnesses,
                size_t nbSelected, RNG::RNG& rng) const
{

    std::set<std::shared_ptr<const Individual>, SharedLess<Individual>> available;
    for(const auto& pair: individualFitnesses) {
        available.insert(pair.second);
    }

    // Standard (mu+lambda) replacement
    std::vector<std::shared_ptr<const Individual>> selected;
    while (available.size() > 0 && selected.size() < nbSelected) {
        // Select random individual
        auto it = available.begin();
        std::advance(it, rng.uniformSample<size_t>(0, available.size() - 1));
        selected.push_back(*it);

        // Remove selected individual if replacement is not allowed.
        if(!this->replacement) {
            available.erase(it);
        }
    }
    return selected;
}