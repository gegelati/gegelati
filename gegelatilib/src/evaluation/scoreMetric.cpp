
#include "evaluation/scoreMetric.h"

std::unique_ptr<Evaluation::EvaluationMetric> Evaluation::ScoreMetric::cloneEmptyUniquePtr() const
{
    return std::make_unique<ScoreMetric>();
}

double Evaluation::ScoreMetric::getScore() const
{
    return score;
}

void Evaluation::ScoreMetric::extractMetricsRun(
    const Evolution::Individual& individual, size_t nbStepsExecuted,
    const Evaluation::LearningEnvironment& learningEnvironment)
{
    // Update score
    this->score = learningEnvironment.getScore();
}
