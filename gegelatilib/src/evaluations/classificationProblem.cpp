
#include "evaluations/classificationProblem.h"
 
std::vector<Data::DataView> Evaluations::ClassificationProblem::getDataViewAt(size_t index) const
{
  if(index > this->dataset.size()) {
    throw std::runtime_error("Evaluations::ClassificationProblem::getDataViewAt: index out of dataset range");
  }
  const std::vector<Data::DataValue>& inputSourcesValue = this->dataset.at(index).first;
  std::vector<Data::DataView> inputSourcesView;

  for(const Data::DataValue& value: inputSourcesValue) {
      inputSourcesView.push_back(value.view());
  }

  return std::move(inputSourcesView);
}

const size_t& Evaluations::ClassificationProblem::getTargetAt(size_t index) const
{
  if(index > this->dataset.size()) {
    throw std::runtime_error("Evaluations::ClassificationProblem::getDataViewAt: index out of dataset range");
  }
  return this->dataset.at(index).second;
}

uint64_t Evaluations::ClassificationProblem::maxHash() const
{
    return this->dataset.size() - 1;
}

void Evaluations::ClassificationProblem::extractFeatures(
  const Individual& individual,
  const std::map<size_t, std::unique_ptr<Feature>>& features,
  const std::set<uint64_t>& hashes) const 
{
  for(const uint64_t& hash: hashes) {

    std::vector<Data::DataView> inputSources = this->getDataViewAt(hash);

    for(const auto& [key, feature]: features) {
      feature->extractBeforeExecution(hash, individual, inputSources, *this);
    }

    Data::DataValue executionReturn = individual.execute(inputSources);

    for(const auto& [key, feature]: features) {
      feature->extractAfterExecution(hash, individual, executionReturn, *this);
    }
  }
}
