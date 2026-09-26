
#include "evaluations/evaluator.h"

std::set<uint64_t> Evaluations::Evaluator::computeEvaluationHashes(
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

void Evaluations::Evaluator::evaluateIndividuals(
  const std::set<std::shared_ptr<const Individual>, SharedLess<Individual>>& individuals, 
  Evaluations::Problem& problem,
  const std::vector<std::shared_ptr<const Metric>>& metrics,
  size_t nbIterations,
  uint64_t generationNumber,
  Mode mode) const
{
  if(metrics.empty() || individuals.empty()) {
    throw std::runtime_error("Evaluations::Evaluator::evaluateIndividuals: cannot evaluate with empty list of metrics or empty set of individuals");
  }

  // Define hashes
  std::set<uint64_t> hashes = this->computeEvaluationHashes(
    nbIterations, generationNumber, mode, problem.getProblemSeed(), problem.maxHash());

  for(const std::shared_ptr<const Individual>& individual: individuals) {

    std::map<size_t, std::unique_ptr<Evaluations::Feature>> features = Metric::getUniqueRequestedMetrics(metrics);

    problem.extractFeatures(*individual, features, hashes);

    individual->addFeatures(std::move(features));
  }
}
