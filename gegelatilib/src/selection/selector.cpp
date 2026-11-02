#include "selection/selector.h"

std::vector<std::pair<uint64_t, double>> Selection::Selector::getOrderedFitness(
    const std::map<uint64_t, double>& fitnessMap, bool ascending)
{
    std::vector<std::pair<uint64_t, double>> orderedFitness(fitnessMap.begin(), fitnessMap.end());
    
    // Sort the individual to get ranks.
    std::stable_sort( orderedFitness.begin(), orderedFitness.end(),
        [ascending](const auto& a, const auto& b) {
            if(a.second != b.second) {
                // First rank by fitness
                return ascending ? a.second < b.second : a.second > b.second;
            } else {
                // If fitness is equal, individual with lower ID is higher: it favors older individuals.
                return ascending ? a.first > b.first : a.first < b.first;
            }
        });

    return std::move(orderedFitness);
}