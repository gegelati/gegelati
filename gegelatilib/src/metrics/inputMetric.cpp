
#include "metrics/inputMetric.h"

std::unique_ptr<Metrics::Metric> Metrics::InputMetric::cloneEmptyPtr() const
{
    return std::make_unique<InputMetric>();
}


void Metrics::InputMetric::extractBeforeExecution(
    const uint64_t& hash, const Individual& individual, 
    const std::vector<Data::DataValue>& inputs, const Evaluation::Problem& problem)
{
    std::vector<Data::DataValue> currentInputs;
    for(const Data::DataValue& input: inputs) {
        currentInputs.push_back(std::move(input.clone()));
    }
    this->inputs.push_back(std::move(currentInputs));
}

std::string Metrics::InputMetric::toString(std::string prefix) const
{
    std::stringstream ss;
    ss << prefix << "InputMetric with "<< this->inputs.size() <<" extractions: \n";
    for(size_t idx=0; idx < this->inputs.size(); idx++) {
        ss << prefix << "  Extraction "<<idx<<": \n";
        for(size_t idxInput = 0; idxInput < this->inputs.at(idx).size(); idxInput++) {
            ss << this->inputs.at(idx).at(idxInput).toString(prefix + "    ") <<",\n";
        }
    }
    return ss.str();
}

uint64_t Metrics::InputMetric::staticHash() 
{
    return std::type_index(typeid(InputMetric)).hash_code();
}

const std::vector<std::vector<Data::DataValue>>& Metrics::InputMetric::getInputs() const
{
    return this->inputs;
}