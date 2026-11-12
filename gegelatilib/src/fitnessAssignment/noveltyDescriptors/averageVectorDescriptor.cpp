#include "fitnessAssignment/noveltyDescriptors/averageVectorDescriptor.h"

#include <stdexcept>

std::vector<double> Descriptors::AverageVectorDescriptor::compute(Metrics::MetricMap& metrics) const
{
    const auto vectorMetrics = metrics.getMetricValues<Metrics::VectorMetric>(this->metricHash);
    if(vectorMetrics.empty()) {
        throw std::runtime_error("Descriptors::AverageVectorDescriptor::compute: no metric with the required hash.");
    }

    auto it = vectorMetrics.begin();
    std::vector<double> average = it->second->getValues();
    it++;
    for(; it != vectorMetrics.end(); it++) {
        const std::vector<double>& currentValues = it->second->getValues();
        if(currentValues.size() != average.size()) {
            throw std::runtime_error("Descriptors::AverageVectorDescriptor::compute: All metrics must have the same number of values.");
        }
        for(size_t idx = 0; idx < average.size(); idx++) {
            average.at(idx) += currentValues.at(idx);
        }
    }

    for(double& value : average) {
        value /= static_cast<double>(vectorMetrics.size());
    }
    return average;
}

std::vector<Dimensions::NumericRange<double>> Descriptors::AverageVectorDescriptor::ranges(Metrics::MetricMap& metrics) const
{
    const auto vectorMetrics = metrics.getMetricValues<Metrics::VectorMetric>(this->metricHash);
    if(vectorMetrics.empty()) {
        throw std::runtime_error("Descriptors::AverageVectorDescriptor::ranges: no metric with the required hash.");
    }

    auto it = vectorMetrics.begin();
    std::vector<Dimensions::NumericRange<double>> result = it->second->getRanges();
    it++;
    for(; it != vectorMetrics.end(); it++) {
        const auto& currentRanges = it->second->getRanges();
        if(currentRanges.size() != result.size()) {
            throw std::runtime_error("Descriptors::AverageVectorDescriptor::ranges: All metrics must have the same number of values.");
        }
        for(size_t idx = 0; idx < result.size(); idx++) {
            result.at(idx) = hull(result.at(idx), currentRanges.at(idx));
        }
    }
    return result;
}
