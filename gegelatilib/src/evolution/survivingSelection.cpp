#include "evolution/survivingSelection.h"

std::vector<std::unique_ptr<Evaluation::EvaluationMetric>> Evolution::SurvivingSelection::getSelectionMetrics()
{
    std::vector<std::unique_ptr<Evaluation::EvaluationMetric>> vect;
    vect.push_back(std::make_unique<Evaluation::EvaluationMetric>());
    return vect;
}

std::vector<std::pair<double, std::reference_wrapper<const Evolution::Individual>>> Evolution::SurvivingSelection::getRankedScores(
    const std::map<std::reference_wrapper<const Individual>, std::shared_ptr<Evaluation::EvaluationResult>>& scores
) const
{
    // Get the average score of each individual.
    std::vector<std::pair<double, std::reference_wrapper<const Individual>>> ranked;
    for (const auto& pairResult : scores){
        double score = 0;
        for (const auto& pairRun: pairResult.second->getEvaluationRuns()) {
            score += pairRun.second->getMetricAt(0).getScore();
        }
        ranked.emplace_back(score, pairResult.first);
    }

    // Sort the individual to get ranks.
    std::stable_sort(ranked.begin(), ranked.end(),
        [](const auto& a, const auto& b) { return a.first > b.first; });

    return ranked;
}

std::map<std::reference_wrapper<const Evolution::Individual>, bool> Evolution::SurvivingSelection::select(
    const std::map<std::reference_wrapper<const Individual>, std::shared_ptr<Evaluation::EvaluationResult>>& scores
) const
{
    std::vector<std::pair<double, std::reference_wrapper<const Evolution::Individual>>> ranked = this->getRankedScores(scores);

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
    return this->getRankedScores(scores).begin()->second;
}