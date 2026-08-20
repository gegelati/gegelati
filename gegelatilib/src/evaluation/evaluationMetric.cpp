
#include "evaluation/evaluationMetric.h"

std::unique_ptr<Evaluation::EvaluationMetric> Evaluation::EvaluationMetric::cloneEmptyUniquePtr() const
{
    return std::make_unique<EvaluationMetric>();
}

double Evaluation::EvaluationMetric::getScore() const
{
    return score;
}

void Evaluation::EvaluationMetric::extractMetricsRun(
    const Evolution::Individual& individual, size_t nbStepsExecuted,
    const Learn::LearningEnvironment& learningEnvironment)
{
    // Update score
    this->score = learningEnvironment.getScore();
}
