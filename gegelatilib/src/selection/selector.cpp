#include "selection/selector.h"

std::vector<std::unique_ptr<Evaluation::EvaluationMetric>> Selection::Selector::getSelectionMetrics()
{
    std::vector<std::unique_ptr<Evaluation::EvaluationMetric>> vect;
    vect.push_back(std::make_unique<Evaluation::ScoreMetric>(0u));
    return vect;
}

std::vector<std::pair<double, std::shared_ptr<const Individual>>> Selection::Selector::assignFitness(
    const std::set<std::shared_ptr<const Individual>, SharedLess<Individual>>& individuals
) const
{
    // Get the average score of each individual.
    std::vector<std::pair<double, std::shared_ptr<const Individual>>> ranked;
    for (const std::shared_ptr<const Individual>& individual : individuals){

        double score = 0;
        const std::vector<std::unique_ptr<Evaluation::EvaluationMetric>>& scoreMetrics = individual->getEvaluationResult().getEvaluationMetricsAt(typeid(Evaluation::ScoreMetric));
        if(scoreMetrics.empty()) {
            throw std::runtime_error("Selection::getRankedScores: No score metric recieved for computing fitness");
        }

        for (const std::unique_ptr<Evaluation::EvaluationMetric>& metric: scoreMetrics) {
            // No check that it is effectively a score metric because we are crazyyy
            score += dynamic_cast<const Evaluation::ScoreMetric*>(metric.get())->getScore();
        }
        score /= static_cast<double>(scoreMetrics.size());
        ranked.emplace_back(score, individual);
    }

    // Sort the individual to get ranks.
    std::stable_sort(ranked.begin(), ranked.end(),
        [](const auto& a, const auto& b) { return a.first > b.first; });

    return ranked;
}

const Individual& Selection::Selector::getBest(
    const std::set<std::shared_ptr<const Individual>, SharedLess<Individual>>& individuals) const
{    
    return *this->assignFitness(individuals).begin()->second;
}