
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

std::string Evaluation::EvaluationRun::toString(std::string prefix) const
{
    std::ostringstream oss;
    if(this->metrics.size() > 1) {
        oss << "EvaluationRun{\n";
        for(size_t idx = 0; idx < this->metrics.size(); idx++) {
            oss << prefix << "\t(" << idx << ") " << this->metrics.at(idx)->toString() << ",\n";
        }
        oss << prefix <<"}";
    } else {
        oss << "EvaluationRun{" << this->metrics.at(0)->toString() << "}";
    }

    return oss.str();

}