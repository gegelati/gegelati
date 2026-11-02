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

    // Individuals of several lists, one after the other.
    std::vector<std::shared_ptr<const Individual>> getAllIndividuals(const Evolution::EvolutionState& state, const std::vector<std::string>& names)
    {
        std::vector<std::shared_ptr<const Individual>> individuals;
        for(const std::string& name: names) {
            const std::vector<std::shared_ptr<const Individual>> list = state.getIndividuals(name);
            individuals.insert(individuals.end(), list.begin(), list.end());
        }
        return individuals;
    }
}


Evolution::EvaluateStep::EvaluateStep(std::vector<std::string> evaluated, std::string metrics, std::shared_ptr<Evaluation::Problem> problem,
                                      std::shared_ptr<const Evaluation::Evaluator> evaluator, std::shared_ptr<const Metrics::MetricMapTemplate> metricTemplate,
                                      size_t nbIterations, Evaluation::Mode mode, std::string name)
    : Step(std::move(name)), evaluated{std::move(evaluated)}, metrics{std::move(metrics)}, problem{std::move(problem)}, evaluator{std::move(evaluator)},
      metricTemplate{std::move(metricTemplate)}, nbIterations{nbIterations}, mode{mode}
{
    if(this->problem == nullptr) {
        throw std::runtime_error("Evolution::EvaluateStep: the problem cannot be null");
    }
}

std::set<std::string> Evolution::EvaluateStep::requiredData() const
{
    return std::set<std::string>(this->evaluated.begin(), this->evaluated.end());
}

std::set<std::string> Evolution::EvaluateStep::providedData() const
{
    return {this->metrics};
}

std::string Evolution::EvaluateStep::details() const
{
    return modeName(this->mode) + ", " + std::to_string(this->nbIterations) + " iterations";
}

void Evolution::EvaluateStep::apply(EvolutionState& state)
{
    state.set(this->metrics, std::make_shared<std::map<uint64_t, std::unique_ptr<Metrics::MetricMap>>>(this->evaluator->evaluateIndividuals(
        getAllIndividuals(state, this->evaluated), *this->problem, this->metricTemplate, this->nbIterations, state.generation, this->mode)));
}
