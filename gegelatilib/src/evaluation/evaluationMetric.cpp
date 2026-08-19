
#include "evaluation/evaluationMetric.h"

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
