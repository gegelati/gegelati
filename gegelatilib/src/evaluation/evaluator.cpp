
#include "evaluation/evaluator.h"
std::map<std::shared_ptr<const Individual>, std::unique_ptr<Metrics::MetricMap>, SharedLess<Individual>> Evaluation::Evaluator::evaluateIndividuals(
  const std::set<std::shared_ptr<const Individual>, SharedLess<Individual>>& individuals, 
  Problem& problem, std::shared_ptr<const Metrics::MetricMapTemplate> metricTemplate,
  size_t nbIterations, uint64_t generationNumber,
  Mode mode) const
{
  if(metricTemplate->size() == 0 || individuals.empty()) {
    throw std::runtime_error("Evaluation::Evaluator::evaluateIndividuals: cannot evaluate with empty list of metrics or empty set of individuals");
  }

  // Define hashes
  std::set<uint64_t> hashes = problem.computeEvaluationHashes(
    nbIterations, generationNumber, mode, problem.getProblemSeed());

  std::map<std::shared_ptr<const Individual>, std::unique_ptr<Metrics::MetricMap>, SharedLess<Individual>> results;
  for(const std::shared_ptr<const Individual>& individual: individuals) {

    std::unique_ptr<Metrics::MetricMap> localMetrics = std::make_unique<Metrics::MetricMap>(metricTemplate);

    problem.extractMetrics(*individual, *localMetrics, hashes);

    results.insert(std::make_pair(individual, std::move(localMetrics)));
  }
  return results;
}
