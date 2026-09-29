
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
    std::stringstream ss;
    ss << prefix << "OutputMetric with "<< this->outputs.size() <<" extractions: \n";
    for(size_t idx=0; idx < this->outputs.size(); idx++) {
        ss << prefix << "  Extraction "<<idx<<": \n";
        ss << this->outputs.at(idx).toString(prefix + "    ") <<",\n";
    }
    return ss.str();
}

uint64_t Metrics::OutputMetric::staticHash() 
{
    return std::type_index(typeid(OutputMetric)).hash_code();
}

const std::vector<Data::DataValue>& Metrics::OutputMetric::getOutputs() const
{
    return this->outputs;
}