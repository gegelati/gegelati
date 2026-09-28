
#include "evaluation/reinforcementProblem.h"
 


void Evaluation::ReinforcementProblem::extractMetrics(
  const Individual& individual, MetricMap& metrics,
  const std::set<uint64_t>& hashes) const 
{
  for(const uint64_t& hash: hashes) {


    // Reset the Environment
    this->env.reset(hash);

    // Perform steps until limit or environment is terminal.
    for(uint64_t idx = 0; idx < this->env.getMaxSteps() && !this->env.isTerminal(); idx++) {

      // Get input sources
      std::vector<Data::DataView> inputSources = this->env.getDataSources();

      // Extract before execution (can be interpreted as "after environment action")
      metrics.extractBeforeExecution(hash, individual, inputSources, *this);

      // Execute individual
      Data::DataValue executionReturn = individual.execute(inputSources);

      // Extract after execution
      metrics.extractAfterExecution(hash, individual, executionReturn, *this);

      // Do environment action
      this->env.doAction(executionReturn);
    }

    // Extract at the end of episode and extract compute metric
    metrics.extractionEndAndComputeMetric(hash, individual, *this);
  }
}

const Evaluation::ReinforcementEnvironment& Evaluation::ReinforcementProblem::getEnvironment() const
{
  return this->env;
}