
#include "evaluation/predictionProblem.h"


std::unique_ptr<Evaluation::Problem> Evaluation::PredictionProblem::cloneUniquePtr() const
{
  return std::make_unique<Evaluation::PredictionProblem>(this->dataset, this->problemSeed);
}

bool Evaluation::PredictionProblem::isCopyable() const
{
  return true;
}

uint64_t Evaluation::PredictionProblem::maxHash() const
{
    return this->dataset.size();
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
