#include "fitnessAssignment/noveltyDescriptors/scalarListDescriptor.h"

std::vector<double> Descriptors::ScalarListDescriptor::compute(Metrics::MetricMap& metrics, uint64_t metricHash) const
{
    std::vector<double> result;
    for(const auto& [featureHash, metric] : metrics.getMetricValues<Metrics::ScalarMetric>(metricHash)) {
        result.push_back(metric->getValue());
    }
    return result;
}

std::vector<Dimensions::NumericRange<double>> Descriptors::ScalarListDescriptor::ranges(Metrics::MetricMap& metrics, uint64_t metricHash) const
{
    std::vector<Dimensions::NumericRange<double>> result;
    for(const auto& [featureHash, metric] : metrics.getMetricValues<Metrics::ScalarMetric>(metricHash)) {
        result.push_back(metric->getRange());
    }
    return result;
}
