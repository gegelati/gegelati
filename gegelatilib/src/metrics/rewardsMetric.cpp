#include "metrics/rewardsMetric.h"

std::unique_ptr<Metrics::Metric> Metrics::RewardsMetric::cloneEmptyPtr() const
{
    return std::make_unique<RewardsMetric>();
}


void Metrics::RewardsMetric::extractBeforeExecution(
    const uint64_t& hash,
    const Individual& individual,
    const std::vector<Data::DataValue>& inputs,
    const Evaluation::Problem& problem)
{
    if(dynamic_cast<const Evaluation::ReinforcementProblem*>(&problem) == nullptr) {
        throw std::runtime_error ("Metrics::RewardsMetric::extractBeforeExecution: feature can only be extracted on a reinforcementProblem");
    }
    this->score += dynamic_cast<const Evaluation::ReinforcementProblem&>(problem).getEnvironment().getLastReward();
    
}

void Metrics::RewardsMetric::extractionEnd(
    const uint64_t& hash,
    const Individual& individual,
    const Evaluation::Problem& problem) 
{
    if(dynamic_cast<const Evaluation::ReinforcementProblem*>(&problem) == nullptr) {
        throw std::runtime_error ("Metrics::RewardsMetric::extractionEndAndComputeMetric: feature can only be extracted on a reinforcementProblem");
    }
    this->score += dynamic_cast<const Evaluation::ReinforcementProblem&>(problem).getEnvironment().getLastReward();
}

std::string Metrics::RewardsMetric::toString(std::string prefix) const
{
    return prefix + " RewardsMetric with score of " + std::to_string(this->score);
}

uint64_t Metrics::RewardsMetric::staticHash() 
{
    return std::type_index(typeid(RewardsMetric)).hash_code();
}