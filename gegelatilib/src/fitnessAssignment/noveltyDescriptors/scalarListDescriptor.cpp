#include "fitnessAssignment/noveltyDescriptors/scalarListDescriptor.h"

std::vector<double> Descriptors::ScalarListDescriptor::compute(Metrics::MetricMap& metrics) const
{
    std::vector<double> result;
    for(const auto& [featureHash, metric] : metrics.getMetricValues<Metrics::ScalarMetric>(this->metricHash)) {
        result.push_back(metric->getValue());
    }
    return result;
}

std::vector<Dimensions::NumericRange<double>> Descriptors::ScalarListDescriptor::ranges(Metrics::MetricMap& metrics) const
{
    std::vector<Dimensions::NumericRange<double>> result;
    for(const auto& [featureHash, metric] : metrics.getMetricValues<Metrics::ScalarMetric>(this->metricHash)) {
        result.push_back(metric->getRange());
    }
    return result;
}
