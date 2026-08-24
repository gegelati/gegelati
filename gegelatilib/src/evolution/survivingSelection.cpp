#include "evolution/survivingSelection.h"

std::vector<std::unique_ptr<Evaluation::EvaluationMetric>> Evolution::SurvivingSelection::getSelectionMetrics()
{
    std::vector<std::unique_ptr<Evaluation::EvaluationMetric>> vect;
    vect.push_back(std::make_unique<Evaluation::ScoreMetric>());
    return vect;
}

std::vector<std::pair<double, std::reference_wrapper<const Evolution::Individual>>> Evolution::SurvivingSelection::getRankedScores(
    const std::set<std::reference_wrapper<const Individual>>& individuals
) const
{
    // Get the average score of each individual.
    std::vector<std::pair<double, std::reference_wrapper<const Individual>>> ranked;
    for (const Individual& individual : individuals){
        double score = 0;
        for (const auto& pairRun: individual.getEvaluationResult().getEvaluationRuns()) {

            // Ugly loop to find the corresponding metric, a set should be considered.
            for(const std::unique_ptr<Evaluation::EvaluationMetric>& metric: pairRun.second->getMetrics()) {
                if(dynamic_cast<Evaluation::ScoreMetric*>(metric.get()) != nullptr) {
                    score += dynamic_cast<Evaluation::ScoreMetric*>(metric.get())->getScore();
                    break;
                }
            }
        }
        ranked.emplace_back(score, individual);
    }

    // Sort the individual to get ranks.
    std::stable_sort(ranked.begin(), ranked.end(),
        [](const auto& a, const auto& b) { return a.first > b.first; });

    return ranked;
}

std::map<std::reference_wrapper<const Evolution::Individual>, bool> Evolution::SurvivingSelection::select(
    const std::set<std::reference_wrapper<const Individual>>& individuals
) const
{
    std::vector<std::pair<double, std::reference_wrapper<const Evolution::Individual>>> ranked = this->getRankedScores(individuals);

    // Standard (mu+lambda) replacement
    std::map<std::reference_wrapper<const Individual>, bool> selection;
    size_t mu = 100;
    for (size_t idx = 0; idx < ranked.size(); ++idx) {
        selection.insert({ranked[idx].second, idx < mu});
    }
    return selection;
}

const Evolution::Individual& Evolution::SurvivingSelection::getBest(
    const std::set<std::reference_wrapper<const Individual>>& individuals) const
{    
    return this->getRankedScores(individuals).begin()->second;
}