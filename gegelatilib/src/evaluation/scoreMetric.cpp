
#include "evaluation/scoreMetric.h"

std::unique_ptr<Evaluation::EvaluationMetric> Evaluation::ScoreMetric::cloneEmptyUniquePtr(size_t seed) const
{
    return std::make_unique<ScoreMetric>(seed);
}

double Evaluation::ScoreMetric::getScore() const
{
    return score;
}

void Evaluation::ScoreMetric::extractMetricRun(
    const Individual& individual, size_t nbStepsExecuted,
    const Evaluation::Problem& problem)
{
    // Update score
    this->score = problem.getScore();
}


std::string Evaluation::ScoreMetric::toString(std::string prefix) const
{
    std::ostringstream oss;
    oss << prefix << "Seed:"<<this->seed <<"; Score:" << score;

    return oss.str();
}
std::type_index Evaluation::ScoreMetric::typeId() const noexcept
{
    return typeid(ScoreMetric);
}
