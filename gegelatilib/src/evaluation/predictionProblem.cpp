
#include "evaluation/predictionProblem.h"

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
  for(const uint64_t& hash: hashes) {

    // Get input sources
    const std::vector<Data::DataValue>& inputSources = this->dataset.getInputsAt(hash);

    // Extract before execution (ex: for time measurement)
    metrics.extractBeforeExecution(hash, individual, inputSources, *this);

    // excute indiviudal
    Data::DataValue executionReturn = individual.execute(inputSources);

    // extract after execution
    metrics.extractAfterExecution(hash, individual, executionReturn, *this);

    // Extract at the end and compute metric
    metrics.extractionEnd(hash, individual, *this);
  }
}
