
#include "evaluation/classificationProblem.h"
 
std::vector<Data::DataView> Evaluation::ClassificationProblem::getDataViewAt(size_t index) const
{
  if(index > this->dataset.size()) {
    throw std::runtime_error("Evaluation::ClassificationProblem::getDataViewAt: index out of dataset range");
  }
  const std::vector<Data::DataValue>& inputSourcesValue = this->dataset.at(index).first;
  std::vector<Data::DataView> inputSourcesView;

  for(const Data::DataValue& value: inputSourcesValue) {
      inputSourcesView.push_back(value.view());
  }

  return std::move(inputSourcesView);
}

const size_t& Evaluation::ClassificationProblem::getTargetAt(size_t index) const
{
  if(index > this->dataset.size()) {
    throw std::runtime_error("Evaluation::ClassificationProblem::getDataViewAt: index out of dataset range");
  }
  return this->dataset.at(index).second;
}

uint64_t Evaluation::ClassificationProblem::maxHash() const
{
    return this->dataset.size() - 1;
}

void Evaluation::ClassificationProblem::extractMetrics(
  const Individual& individual, MetricMap& metrics,
  const std::set<uint64_t>& hashes) const 
{
  for(const uint64_t& hash: hashes) {

    // Get input sources
    std::vector<Data::DataView> inputSources = this->getDataViewAt(hash);

    // Extract before execution (ex: for time measurement)
    metrics.extractBeforeExecution(hash, individual, inputSources, *this);

    // excute indiviudal
    Data::DataValue executionReturn = individual.execute(inputSources);

    // extract after execution
    metrics.extractAfterExecution(hash, individual, executionReturn, *this);

    // Extract at the end and compute metric
    metrics.extractionEndAndComputeMetric(hash, individual, *this);
  }
}
