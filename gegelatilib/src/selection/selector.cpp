#include "selection/selector.h"

const std::vector<std::shared_ptr<const Evaluations::Metric>>& Selection::Selector::getSelectionMetrics()
{
    return this->metrics;
}

std::vector<std::pair<double, std::shared_ptr<const Individual>>> Selection::Selector::assignFitness(
    const std::set<std::shared_ptr<const Individual>, SharedLess<Individual>>& individuals
) const
{
    // Get the average score of each individual.
    std::vector<std::pair<double, std::shared_ptr<const Individual>>> ranked;
    for (const std::shared_ptr<const Individual>& individual : individuals){

        double score = this->metrics.at(0)->computeMetrics(individual->getFeatures()).getScalar<double>();
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