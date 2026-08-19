#include "evolution/survivingSelection.h"

std::map<std::reference_wrapper<const Evolution::Individual>, bool> Evolution::SurvivingSelection::select(const std::map<std::reference_wrapper<const Individual>, std::shared_ptr<Learn::EvaluationResult>>& scores) const
{
    std::vector<std::pair<double, std::reference_wrapper<const Individual>>> ranked;
    for (const auto& [ind, eval] : scores)
        ranked.emplace_back(eval->getSelectionMetrics()->getScore(), ind);

    // Sorting ranks
    std::stable_sort(ranked.begin(), ranked.end(),
        [](const auto& a, const auto& b) { return a.first > b.first; });

    // Standard (mu+lambda) replacement
    std::map<std::reference_wrapper<const Individual>, bool> selection;
    size_t mu = 100;
    for (size_t idx = 0; idx < ranked.size(); ++idx) {
        selection.insert({ranked[idx].second, idx < mu});
    }
    return selection;
}

std::pair<std::reference_wrapper<const Evolution::Individual>, std::shared_ptr<Learn::EvaluationResult>> Evolution::SurvivingSelection::getBest(
    const std::map<std::reference_wrapper<const Individual>, std::shared_ptr<Learn::EvaluationResult>>& scores) const
{
    auto it = scores.begin();
    std::pair<std::reference_wrapper<const Evolution::Individual>, std::shared_ptr<Learn::EvaluationResult>> bestIndiv = *it;
    it++;

    while(it != scores.end()) {
        if(it->second->getSelectionMetrics()->getScore() > bestIndiv.second->getSelectionMetrics()->getScore()) {
            bestIndiv = *it;
        }    
        it++;
    }
    return bestIndiv;
}