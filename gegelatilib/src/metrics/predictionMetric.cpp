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

    //this->prediction = std::make_shared<const Data::DataValue>(output.clone());
    //this->target = std::make_shared<const Data::DataValue>(classifProblem.getDataSet().getOutputAt(hash).clone());
    //this->values[0] = double(*this->prediction == *target);
    this->values[0] = double(output == classifProblem.getDataSet().getOutputAt(hash));
}

std::string Metrics::PredictionMetric::toString(std::string prefix) const
{
    return prefix + " PredictionMetric with success: " + std::to_string(this->values[0]);
}

const Data::DataValue& Metrics::PredictionMetric::getPrediction() const
{
    return *this->prediction;
}

const Data::DataValue& Metrics::PredictionMetric::getTarget() const
{
    return *this->target;
}


uint64_t Metrics::PredictionMetric::staticHash() 
{
    return std::type_index(typeid(PredictionMetric)).hash_code();
}