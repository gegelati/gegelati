
#include "fitnessAssignment/defaultAssigner.h"

std::vector<std::pair<double, std::shared_ptr<const Individual>>> FitnessAssignment::DefaultAssigner::assignFitness(
                const std::map<std::shared_ptr<const Individual>, std::unique_ptr<Evaluation::MetricMap>, SharedLess<Individual>>& individualMetrics,
                uint64_t metricHash) const
{
    // Get the average score of each individual.
    std::vector<std::pair<double, std::shared_ptr<const Individual>>> ranked;
    for (const auto& [individual, metricMap] : individualMetrics){

        double avgScore = 0.0;

        const Evaluation::Metric& metric = *metricMap->metrics.at(metricHash);
        for(const uint64_t& key: metric.getKeys()) {
            avgScore += metric.getMetricAt(key).getScalar<double>();
        }

        avgScore /= metric.size();
        
        ranked.emplace_back(avgScore, individual);
    }

    // Sort the individual to get ranks.
    std::stable_sort(ranked.begin(), ranked.end(),
        [](const auto& a, const auto& b) { return a.first > b.first; });

    return ranked;
}