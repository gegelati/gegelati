
#include "evaluation/predictionProblem.h"


std::unique_ptr<Evaluation::Problem> Evaluation::PredictionProblem::cloneUniquePtr() const
{
  return std::make_unique<Evaluation::PredictionProblem>(this->dataset, this->problemSeed, this->getName());
}

bool Evaluation::PredictionProblem::isCopyable() const
{
  return true;
}

std::set<uint64_t> Evaluation::PredictionProblem::computeEvaluationHashes(
  uint64_t nbIterations, uint64_t generationNumber, Mode mode,
  uint64_t problemSeed) const
{
  std::set<uint64_t> hashes;

  // Samples are drawn in the data of the mode, so there must be some.
  if(nbIterations > 0 && ((mode == Mode::TRAINING) ? this->dataset.sizeTraining() : this->dataset.sizeValidation()) == 0) {
    throw std::runtime_error(std::string("Evaluation::PredictionProblem: the dataset has no ") + ((mode == Mode::TRAINING) ? "training" : "validation") + " data to evaluate on");
  }

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
    // Avoid the same hashes
    if(mode == Mode::TRAINING) {
      hashes.insert(hash % this->dataset.sizeTraining());
    } else {
      hashes.insert((hash % this->dataset.sizeValidation()) + this->dataset.sizeTraining());
    }
  }

  return hashes;
}
const Evaluation::DataSet& Evaluation::PredictionProblem::getDataSet() const
{
  return this->dataset;
}

void Evaluation::PredictionProblem::extractMetrics(
  const Individual& individual, Metrics::MetricMap& metrics,
  const std::set<uint64_t>& hashes) const 
{
  Data::Hash<uint64_t> hasher;
  uint64_t evaluationHash = 0;

  for(const uint64_t& hash: hashes) {
    evaluationHash ^= hasher(hash);

    // Get input sources
    const std::vector<Data::DataValue>& inputSources = this->dataset.getInputsAt(hash);

    // Extract before execution (ex: for time measurement)
    metrics.extractBeforeExecution(hash, individual, inputSources, *this);

    // excute indiviudal
    Data::DataValue executionReturn = individual.execute(inputSources);

    // extract after execution
    metrics.extractAfterExecution(hash, individual, executionReturn, *this);
  }

  // A single metric covers the whole loop over the hashes.
  metrics.extractionEnd(evaluationHash, individual, *this);
}
