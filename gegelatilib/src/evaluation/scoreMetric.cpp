
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
    const Individual& individual, size_t nbStepsExecuted,
    const Evaluation::LearningEnvironment& learningEnvironment)
{
    // Update score
    this->score = learningEnvironment.getScore();
}


std::string Evaluation::ScoreMetric::toString(std::string prefix) const
{
    std::ostringstream oss;
    oss << prefix << "Score: " << score;

    return oss.str();
}

bool Evaluation::ScoreMetric::sameMetricMethod(const EvaluationMetric& other) const
{
    return dynamic_cast<const ScoreMetric*>(&other) != nullptr;
}