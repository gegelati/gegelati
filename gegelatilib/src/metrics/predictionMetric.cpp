#include "metrics/predictionMetric.h"

std::unique_ptr<Metrics::Metric> Metrics::PredictionMetric::cloneEmptyPtr() const
{
    return std::make_unique<PredictionMetric>();
}

void Metrics::PredictionMetric::extractAfterExecution(
    const uint64_t& hash,
    const Individual& individual, 
    const Data::DataValue& output,
    const Evaluation::Problem& problem)
{
    if(dynamic_cast<const Evaluation::PredictionProblem*>(&problem) == nullptr) {
        throw std::runtime_error ("Metrics::PredictionMetric::extractionEndAndComputeMetric: feature can only be extracted on a classification problem");
    }
    const Evaluation::PredictionProblem& classifProblem = dynamic_cast<const Evaluation::PredictionProblem&>(problem);

    const Data::DataValue& target = classifProblem.getDataSet().getOutputAt(hash);
    if(output == target) {
        this->nbSuccess++;
    }
    this->pairs.emplace_back(output.clone(), target.clone());
    this->values[0] = double(this->nbSuccess) / double(this->pairs.size());
}

std::string Metrics::PredictionMetric::toString(std::string prefix) const
{
    return prefix + " PredictionMetric with " + std::to_string(this->pairs.size()) 
        + " predictions and accuracy: " + std::to_string(this->values[0]);
}

const std::vector<std::pair<Data::DataValue, Data::DataValue>>& Metrics::PredictionMetric::getPairs() const
{
    return this->pairs;
}


uint64_t Metrics::PredictionMetric::staticHash() 
{
    return std::type_index(typeid(PredictionMetric)).hash_code();
}