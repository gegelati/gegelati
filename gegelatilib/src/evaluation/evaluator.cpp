
#include "evaluation/evaluator.h"

std::set<uint64_t> Evaluation::Evaluator::computeEvaluationHashes(
  uint64_t nbIterations, uint64_t generationNumber, Mode mode,
  uint64_t problemSeed, uint64_t problemMaxHash) const
{
  std::set<uint64_t> hashes;

  // Compute baseHash
  Data::Hash<uint64_t> hasher;
  uint64_t baseHash = hasher(static_cast<int>(mode)) ^ hasher(problemSeed);
  if(mode == Mode::TRAINING) { 
      // In training, hash should take into consideration the generation number, else not (we don't want validation to change between generations).
      baseHash = hasher(generationNumber) ^ baseHash;
  }

  // Compute hash of each iteration.
  for(size_t idx = 0; idx < nbIterations; idx++) {
    uint64_t hash = hasher(idx) ^ baseHash;
    hashes.insert(hash % problemMaxHash);
  }

  return hashes;
}

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
  std::set<uint64_t> hashes = this->computeEvaluationHashes(
    nbIterations, generationNumber, mode, problem.getProblemSeed(), problem.maxHash());

  std::map<std::shared_ptr<const Individual>, std::unique_ptr<Metrics::MetricMap>, SharedLess<Individual>> results;
  for(const std::shared_ptr<const Individual>& individual: individuals) {

    std::unique_ptr<Metrics::MetricMap> localMetrics = std::make_unique<Metrics::MetricMap>(metricTemplate);

    problem.extractMetrics(*individual, *localMetrics, hashes);

    results.insert(std::make_pair(individual, std::move(localMetrics)));
  }
  return results;
}
