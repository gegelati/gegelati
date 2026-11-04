#include "metrics/inputMetric.h"

#include <sstream>
#include <typeindex>

std::unique_ptr<Metrics::Metric> Metrics::InputMetric::cloneEmptyPtr() const
{
    return std::make_unique<InputMetric>();
}

void Metrics::InputMetric::extractBeforeExecution(const uint64_t& hash, const Individual& individual, const std::vector<Data::DataValue>& inputs, const Evaluation::Problem& problem)
{
    (void)hash;
    (void)individual;
    (void)problem;

    // The inputs may change after the execution (e.g. an environment), so they are copied.
    std::vector<Data::DataValue> copy;
    copy.reserve(inputs.size());
    for(const Data::DataValue& input: inputs) {
        copy.push_back(input.clone());
    }
    this->inputs.push_back(std::move(copy));
}

std::string Metrics::InputMetric::toString(std::string prefix) const
{
    std::stringstream ss;
    ss << prefix << "InputMetric with " << this->inputs.size() << " extractions:\n";
    for(std::size_t idx = 0; idx < this->inputs.size(); ++idx) {
        ss << prefix << "  Extraction " << idx << ":\n";
        for(const Data::DataValue& input: this->inputs[idx]) {
            ss << input.toShortString(prefix + "    ") << ",\n";
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
