#include "fitnessAssignment/noveltyDescriptors/scalarListDescriptor.h"

std::vector<double> Descriptors::ScalarListDescriptor::compute(Metrics::MetricMap& metrics, uint64_t metricHash) const
{
    std::vector<double> result;
    const std::map<uint64_t, const Metrics::ScalarMetric*> scalarMetrics = metrics.getMetricValues<Metrics::ScalarMetric>(metricHash);
    for(const auto& [featureHash, metric] : scalarMetrics) {
        result.push_back(metric->getValue());
    }
    return result;
}

std::vector<Dimensions::NumericRange<double>> Descriptors::ScalarListDescriptor::ranges(Metrics::MetricMap& metrics, uint64_t metricHash) const
{
    std::vector<Dimensions::NumericRange<double>> result;
    const std::map<uint64_t, const Metrics::ScalarMetric*> scalarMetrics = metrics.getMetricValues<Metrics::ScalarMetric>(metricHash);
    for(const auto& [featureHash, metric] : scalarMetrics) {
        result.push_back(metric->getRange());
    }
    return result;
}
