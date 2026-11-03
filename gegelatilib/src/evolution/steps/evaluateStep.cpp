#include "evolution/steps/evaluateStep.h"

#include <stdexcept>

namespace {
    std::string modeName(Evaluation::Mode mode)
    {
        switch(mode) {
            case Evaluation::Mode::TRAINING: return "training";
            case Evaluation::Mode::VALIDATION: return "validation";
            default: return "testing";
        }
    }
}


Evolution::EvaluateStep::EvaluateStep(std::vector<Key<std::vector<std::shared_ptr<Individual>>>> evaluated, Key<std::map<uint64_t, std::unique_ptr<Metrics::MetricMap>>> metrics, std::shared_ptr<Evaluation::Problem> problem,
                                      std::shared_ptr<const Evaluation::Evaluator> evaluator, std::shared_ptr<const Metrics::MetricMapTemplate> metricTemplate,
                                      size_t nbIterations, Evaluation::Mode mode, std::string name)
    : Module(std::move(name)), evaluated{std::move(evaluated)}, metrics{std::move(metrics)}, problem{std::move(problem)}, evaluator{std::move(evaluator)},
      metricTemplate{std::move(metricTemplate)}, nbIterations{nbIterations}, mode{mode}
{
    if(this->problem == nullptr) {
        throw std::runtime_error("Evolution::EvaluateStep: the problem cannot be null");
    }
}

std::set<std::string> Evolution::EvaluateStep::requiredData() const
{
    std::set<std::string> required;
    for(const Key<std::vector<std::shared_ptr<Individual>>>& key: this->evaluated) {
        required.insert(key.name());
    }
    return required;
}

std::set<std::string> Evolution::EvaluateStep::providedData() const
{
    return {this->metrics.name()};
}

std::string Evolution::EvaluateStep::details() const
{
    return modeName(this->mode) + ", " + std::to_string(this->nbIterations) + " iterations";
}

void Evolution::EvaluateStep::apply(EvolutionData& state)
{
    state.set(this->metrics, std::make_shared<std::map<uint64_t, std::unique_ptr<Metrics::MetricMap>>>(this->evaluator->evaluateIndividuals(
        state.getIndividuals(this->evaluated), *this->problem, this->metricTemplate, this->nbIterations, state.generation, this->mode)));
}
