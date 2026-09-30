#include "selection/selector.h"

std::vector<std::pair<std::shared_ptr<const Individual>, double>> Selection::Selector::getOrderedFitness(
    const std::map<std::shared_ptr<const Individual>, double, SharedLess<Individual>>& fitnessMap, bool ascending)
{
    std::vector<std::pair<std::shared_ptr<const Individual>, double>> orderedFitness;
    for (const auto& [indiv, fitness]: fitnessMap) {
        orderedFitness.push_back(std::make_pair(indiv, fitness));
    }
    
    // Sort the individual to get ranks.
    std::stable_sort( orderedFitness.begin(), orderedFitness.end(),
        [ascending](const auto& a, const auto& b) {
            return ascending ? a.second < b.second : a.second > b.second;
        });

    return std::move(orderedFitness);
}