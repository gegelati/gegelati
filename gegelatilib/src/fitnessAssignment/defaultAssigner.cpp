
#include "fitnessAssignment/defaultAssigner.h"


std::map<std::shared_ptr<const Individual>, double, SharedLess<Individual>> FitnessAssignment::DefaultAssigner::assignFitness(
                const std::map<std::shared_ptr<const Individual>, std::unique_ptr<Metrics::MetricMap>, SharedLess<Individual>>& individualMetrics,
                uint64_t metricHash) const
{
    // Get the average score of each individual.
    std::map<std::shared_ptr<const Individual>, double, SharedLess<Individual>> fitnesses;
    for (const auto& [individual, metricMap] : individualMetrics){

        double avgFitness = 0.0;

        std::map<uint64_t, const Metrics::ScalarMetric*> scalarMetrics = metricMap->getMetricValues<Metrics::ScalarMetric>(metricHash);
        for(const auto& [key, metric]: scalarMetrics) {
            avgFitness += metric->getValue();
        }

        avgFitness /= scalarMetrics.size();
        
        fitnesses.insert(std::make_pair(individual, avgFitness));
    }

    return std::move(fitnesses);
}