
#include "evaluations/reinforcementProblem.h"
 


void Evaluations::ReinforcementProblem::extractFeatures(
  const Individual& individual,
  const std::map<size_t, std::unique_ptr<Feature>>& features,
  const std::set<uint64_t>& hashes) const 
{
  for(const uint64_t& hash: hashes) {

    // Reset the Environment
    this->env.reset(hash);

    // Perform steps until limit or environment is terminal.
    for(uint64_t idx = 0; idx < this->env.getMaxSteps() && !this->env.isTerminal(); idx++) {

      std::vector<Data::DataView> inputSources = this->env.getDataSources();

      for(const auto& [key, feature]: features) {
        feature->extractBeforeExecution(hash, individual, inputSources, *this);
      }

      Data::DataValue executionReturn = individual.execute(inputSources);

      for(const auto& [key, feature]: features) {
        feature->extractAfterExecution(hash, individual, executionReturn, *this);
      }

      this->env.doAction(executionReturn);
    }
    for(const auto& [key, feature]: features) {
      feature->extractBeforeExecution(hash, individual, {}, *this);
    }
  }
}

const Evaluations::ReinforcementEnvironment& Evaluations::ReinforcementProblem::getEnvironment() const
{
  return this->env;
}