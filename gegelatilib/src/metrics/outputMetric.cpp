
#include "metrics/outputMetric.h"

std::unique_ptr<Metrics::Metric> Metrics::OutputMetric::cloneEmptyPtr() const
{
    return std::make_unique<OutputMetric>();
}


void Metrics::OutputMetric::extractAfterExecution(
    const uint64_t& hash, const Individual& individual, 
    const Data::DataValue& output, const Evaluation::Problem& problem)
{
    this->outputs.push_back(output.clone());
}

std::string Metrics::OutputMetric::toString(std::string prefix) const
{
    return "todo";
}