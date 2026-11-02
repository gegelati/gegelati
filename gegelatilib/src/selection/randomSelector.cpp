

#include "selection/randomSelector.h"

std::vector<uint64_t> Selection::RandomSelector::select(
                const std::map<uint64_t, double>& fitnessMap,
                size_t nbSelected, RNG::RNG& rng) const
{
    // Who cares about fitness? Only the IDs (ordered) are used.
    std::vector<uint64_t> available;
    available.reserve(fitnessMap.size());
    for(const auto& pair: fitnessMap) {
        available.push_back(pair.first);
    }

    std::vector<uint64_t> selected;
    while (available.size() > 0 && selected.size() < nbSelected) {
        // Select random individual
        const size_t idx = rng.uniformSample<size_t>(0, available.size() - 1);
        selected.push_back(available.at(idx));

        // Remove selected individual if replacement is not allowed.
        if(!this->replacement) {
            available.erase(available.begin() + idx);
        }
    }
    return selected;
}