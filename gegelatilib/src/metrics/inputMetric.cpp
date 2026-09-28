
#include "metrics/inputMetric.h"

std::unique_ptr<Metrics::Metric> Metrics::InputMetric::cloneEmptyPtr() const
{
    return std::make_unique<InputMetric>();
}


void Metrics::InputMetric::extractBeforeExecution(
    const uint64_t& hash, const Individual& individual, 
    const std::vector<Data::DataValue>& inputs, const Evaluation::Problem& problem)
{
    for(const Data::DataValue& input: inputs) {
        this->inputs.push_back(std::move(input.clone()));
    }
}

std::string Metrics::InputMetric::toString(std::string prefix) const
{
    return prefix + "todo";
}