#include "evolution/steps/evaluateStep.h"

#include <stdexcept>


Evolution::EvaluateStep::EvaluateStep(std::vector<std::string> evaluated, std::string metrics, std::shared_ptr<Evaluation::Problem> problem,
                                      std::shared_ptr<const Metrics::MetricMapTemplate> metricTemplate, size_t nbIterations, Evaluation::Mode mode,
                                      size_t nbThreads, std::string name)
    : Module(std::move(name)), evaluated{std::move(evaluated)}, metrics{std::move(metrics)}, problem{std::move(problem)},
      metricTemplate{std::move(metricTemplate)}, nbIterations{nbIterations}, mode{mode}, nbThreads{nbThreads}
{
    for(const std::string& key: this->evaluated) {
        this->require(key);
    }
    this->provide(this->metrics);
    if(this->problem == nullptr) {
        throw std::runtime_error("Evolution::EvaluateStep: the problem cannot be null");
    }
}

std::string Evolution::EvaluateStep::details() const
{
    return Evaluation::modeName(this->mode) + ", " + std::to_string(this->nbIterations) + " iterations, " + this->problem->getProblemName() + ", " + std::to_string(this->nbThreads) + " thread(s)";
}

void Evolution::EvaluateStep::apply(EvolutionData& state)
{
    // The evaluated lists are evaluated together, as individuals that cannot be modified.
    std::vector<std::shared_ptr<const Individual>> individuals;
    for(const std::string& key: this->evaluated) {
        const std::vector<std::shared_ptr<Individual>>& list = *state.get<std::vector<std::shared_ptr<Individual>>>(key);
        individuals.insert(individuals.end(), list.begin(), list.end());
    }
    state.set(this->metrics, std::make_shared<std::map<uint64_t, std::unique_ptr<Metrics::MetricMap>>>(Evaluation::evaluateIndividuals(
        individuals, *this->problem, this->metricTemplate, this->nbIterations, state.generation, this->mode, this->nbThreads)));
}
