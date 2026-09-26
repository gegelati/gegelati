#include "evaluations/predictionFeature.h"

void Evaluations::PredictionFeature::extractAfterExecution(
    const uint64_t& hash,
    const Individual& individual, 
    const Data::DataValue& output,
    const Problem& problem)
{
    const ClassificationProblem& classifProblem = dynamic_cast<const ClassificationProblem&>(problem);

    size_t prediction = output.getScalar<size_t>();
    size_t target = classifProblem.getTargetAt(hash);

    this->features.insert(std::make_pair(hash, Data::DataValue::scalar<bool>(prediction == target)));
}

std::string Evaluations::PredictionFeature::toString(std::string prefix) const
{
    return prefix + "todo";
}