
#include "fitnessAssignment/defaultAssigner.h"


std::map<uint64_t, double> FitnessAssignment::DefaultAssigner::assignFitness(
                const std::map<uint64_t, std::unique_ptr<Metrics::MetricMap>>& individualMetrics) const
{
    // Get the average score of each individual.
    std::map<uint64_t, double> fitnesses;
    for (const auto& [individualID, metricMap] : individualMetrics){

        double avgFitness = 0.0;

        std::map<uint64_t, const Metrics::ScalarMetric*> scalarMetrics = metricMap->getMetricValues<Metrics::ScalarMetric>(this->metricHash);
        for(const auto& [key, metric]: scalarMetrics) {
            avgFitness += metric->getValue();
        }

        avgFitness /= scalarMetrics.size();
        
        fitnesses.insert(std::make_pair(individualID, avgFitness));
    }

    return std::move(fitnesses);
}