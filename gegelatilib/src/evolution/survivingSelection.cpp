#include "evolution/survivingSelection.h"

std::map<std::reference_wrapper<const Evolution::Individual>, bool> Evolution::SurvivingSelection::select(const std::map<std::reference_wrapper<const Individual>, std::shared_ptr<Evaluation::EvaluationResult>>& scores) const
{
    // Probably could update this for the better.
    std::vector<std::pair<double, std::reference_wrapper<const Individual>>> ranked;
    for (const auto& pairResult : scores){
        double score = 0;
        for (const auto& pairRun: pairResult.second->getEvaluationRuns()) {
            score += pairRun.second->getMetricAt(0).getScore();
        }
        ranked.emplace_back(score, pairResult.first);
    }

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

const Evolution::Individual& Evolution::SurvivingSelection::getBest(
    const std::map<std::reference_wrapper<const Individual>, std::shared_ptr<Evaluation::EvaluationResult>>& scores) const
{
    auto it = scores.begin();
    std::pair<std::reference_wrapper<const Evolution::Individual>, std::shared_ptr<Evaluation::EvaluationResult>> bestIndiv = *it;
    it++;


    // Probably could update this for the better.
    std::vector<std::pair<double, std::reference_wrapper<const Individual>>> ranked;
    for (const auto& pairResult : scores){
        double score = 0;
        for (const auto& pairRun: pairResult.second->getEvaluationRuns()) {
            score += pairRun.second->getMetricAt(0).getScore();
        }
        ranked.emplace_back(score, pairResult.first);
    }

    // Sorting ranks
    std::stable_sort(ranked.begin(), ranked.end(),
        [](const auto& a, const auto& b) { return a.first > b.first; });

    
    return ranked.begin()->second;
}