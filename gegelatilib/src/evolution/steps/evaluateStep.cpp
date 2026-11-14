#include "evolution/steps/evaluateStep.h"

#include <stdexcept>


Evolution::EvaluateStep::EvaluateStep(std::vector<std::string> evaluated, std::string metrics, std::string problem,
                                      std::string metricTemplate, size_t nbIterations, Evaluation::Mode mode,
                                      size_t nbThreads, std::string name)
    : Module(std::move(name)), evaluated{std::move(evaluated)}, metrics{std::move(metrics)}, problem{std::move(problem)},
      metricTemplate{std::move(metricTemplate)}, nbIterations{nbIterations}, mode{mode}, nbThreads{nbThreads}
{
    for(const std::string& key: this->evaluated) {
        this->require(key);
    }
    this->require(this->problem);
    this->require(this->metricTemplate);
    this->provide(this->metrics);
}

std::string Evolution::EvaluateStep::details() const
{
    return std::to_string(this->nbIterations) + " iterations, " + std::to_string(this->nbThreads) + " thread(s)";
}

void Evolution::EvaluateStep::apply(EvolutionData& state)
{
    // The evaluated lists are evaluated together, as individuals that cannot be modified.
    std::vector<std::shared_ptr<const Individual>> individuals;
    for(const std::string& key: this->evaluated) {
        const std::vector<std::shared_ptr<Individual>>& list = *state.get<std::vector<std::shared_ptr<Individual>>>(key);
        individuals.insert(individuals.end(), list.begin(), list.end());
    }
    Evaluation::Problem& usedProblem = *state.get<Evaluation::Problem>(this->problem);
    const std::shared_ptr<Metrics::MetricMapTemplate> usedTemplate = state.get<Metrics::MetricMapTemplate>(this->metricTemplate);
    state.set(this->metrics, std::make_shared<std::map<uint64_t, std::unique_ptr<Metrics::MetricMap>>>(Evaluation::evaluateIndividuals(
        individuals, usedProblem, usedTemplate, this->nbIterations, state.generation, this->mode, this->nbThreads)));
}
