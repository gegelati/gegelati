
#include "evaluation/evaluationRun.h"




void Evaluation::EvaluationRun::addMetric(std::unique_ptr<EvaluationMetric> metric)
{
    this->metrics.push_back(std::move(metric));
}
size_t Evaluation::EvaluationRun::getSize() const
{
    return this->metrics.size();
}

const std::vector<std::unique_ptr<Evaluation::EvaluationMetric>>& Evaluation::EvaluationRun::getMetrics() const
{
    return this->metrics;
}

const Evaluation::EvaluationMetric& Evaluation::EvaluationRun::getMetricAt(size_t index) const
{
    if(index >= this->metrics.size()) {
        throw std::runtime_error("Evaluation::EvaluationRun::getMetricAt: index out of range.");
    }
    return *this->metrics.at(index);
}
