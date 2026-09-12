#include "evolution/selection.h"

std::vector<std::unique_ptr<Evaluation::EvaluationMetric>> Evolution::Selection::getSelectionMetrics()
{
    std::vector<std::unique_ptr<Evaluation::EvaluationMetric>> vect;
    vect.push_back(std::make_unique<Evaluation::ScoreMetric>());
    return vect;
}

std::vector<std::pair<double, std::shared_ptr<const Evolution::Individual>>> Evolution::Selection::getRankedScores(
    const std::set<std::shared_ptr<const Individual>, SharedLess<Evolution::Individual>>& individuals
) const
{
    // Get the average score of each individual.
    std::vector<std::pair<double, std::shared_ptr<const Individual>>> ranked;
    for (const std::shared_ptr<const Individual>& individual : individuals){
        // TODO BETTER DESIGN
        size_t nbScoreMetrics = 0;
        double score = 0;
        for (const auto& pairRun: individual->getEvaluationResult().getEvaluationRuns()) {

            // Ugly loop to find the corresponding metric, a set should be considered.
            for(const std::unique_ptr<Evaluation::EvaluationMetric>& metric: pairRun.second->getMetrics()) {
                if(dynamic_cast<Evaluation::ScoreMetric*>(metric.get()) != nullptr) {
                    score += dynamic_cast<Evaluation::ScoreMetric*>(metric.get())->getScore();
                    nbScoreMetrics ++;
                    break;
                }
            }
        }
        ranked.emplace_back(score, individual);

        if(nbScoreMetrics == 0) {
            throw std::runtime_error("Evolution::Selection::getRankedScores: No score metric recieved for computing fitness");
        }
    }

    // Sort the individual to get ranks.
    std::stable_sort(ranked.begin(), ranked.end(),
        [](const auto& a, const auto& b) { return a.first > b.first; });

    return ranked;
}

const Evolution::Individual& Evolution::Selection::getBest(
    const std::set<std::shared_ptr<const Individual>, SharedLess<Evolution::Individual>>& individuals) const
{    
    return *this->getRankedScores(individuals).begin()->second;
}