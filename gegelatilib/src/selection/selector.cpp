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
            if(a.second != b.second) {
                // First rank by fitness
                return ascending ? a.second < b.second : a.second > b.second;
            } else {
                // If fitness is equal, individual with higher ID is higher: it favor newer individuals.
                return ascending ? a.first->getIndividualID() > b.first->getIndividualID() : a.first->getIndividualID() < b.first->getIndividualID();
            }
        });

    return std::move(orderedFitness);
}