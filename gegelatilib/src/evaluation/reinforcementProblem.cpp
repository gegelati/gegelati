
#include "evaluation/reinforcementProblem.h"

#include "metrics/rewardsMetric.h"


std::unique_ptr<Evaluation::Problem> Evaluation::ReinforcementProblem::cloneUniquePtr() const
{
  if(this->env->isCopyable()) {
    return std::make_unique<Evaluation::ReinforcementProblem>(this->env->cloneSharedPtr(), this->problemSeed, this->getName());
  } else {
    throw std::runtime_error("Evaluation::ReinforcementProblem::cloneUniquePtr: Cannot clone problem with unclonable environment");
  }
}

bool Evaluation::ReinforcementProblem::isCopyable() const
{
  return this->env->isCopyable();
}


std::unique_ptr<Metrics::Metric> Evaluation::ReinforcementProblem::getDefaultFitnessMetric() const
{
  return std::make_unique<Metrics::RewardsMetric>();
}

void Evaluation::ReinforcementProblem::extractMetrics(
  const Individual& individual, Metrics::MetricMap& metrics,
  const std::set<uint64_t>& hashes) const 
{
  for(const uint64_t& hash: hashes) {


    // Reset the Environment
    this->env->reset(hash);

    // Perform steps until limit or environment is terminal.
    for(uint64_t idx = 0; idx < this->env->getMaxSteps() && !this->env->isTerminal(); idx++) {

      // Get input sources
      const std::vector<Data::DataValue>& inputSources = this->env->getDataSources();

      // Extract before execution (can be interpreted as "after environment action")
      metrics.extractBeforeExecution(hash, individual, inputSources, *this);

      // Execute individual
      Data::DataValue executionReturn = individual.execute(inputSources);

      // Extract after execution
      metrics.extractAfterExecution(hash, individual, executionReturn, *this);

      // Do environment action
      this->env->doAction(executionReturn);
    }

    // Extract at the end of episode and extract compute metric
    metrics.extractionEnd(hash, individual, *this);
  }
}

const Evaluation::ReinforcementEnvironment& Evaluation::ReinforcementProblem::getEnvironment() const
{
  return *this->env;
}