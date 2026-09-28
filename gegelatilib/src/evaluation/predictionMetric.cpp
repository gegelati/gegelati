#include "evaluation/predictionMetric.h"

std::unique_ptr<Evaluation::Metric> Evaluation::PredictionMetric::cloneEmptyPtr() const
{
    return std::make_unique<PredictionMetric>();
}

void Evaluation::PredictionMetric::extractAfterExecution(
    const uint64_t& hash,
    const Individual& individual, 
    const Data::DataValue& output,
    const Problem& problem)
{
    this->currentPrediction = output.getScalar<size_t>();
}

void Evaluation::PredictionMetric::extractionEndAndComputeMetric(
    const uint64_t& hash,
    const Individual& individual, 
    const Problem& problem)
{
    const ClassificationProblem& classifProblem = dynamic_cast<const ClassificationProblem&>(problem);

    size_t target = classifProblem.getTargetAt(hash);
    this->metrics.insert(std::make_pair(hash, Data::DataValue::scalar<bool>(this->currentPrediction == target)));
}

std::string Evaluation::PredictionMetric::toString(std::string prefix) const
{
    return prefix + "todo";
}