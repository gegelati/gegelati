#include "evaluation/rewardsMetric.h"

std::unique_ptr<Evaluation::Metric> Evaluation::RewardsMetric::cloneEmptyPtr() const
{
    return std::make_unique<RewardsMetric>();
}


void Evaluation::RewardsMetric::extractBeforeExecution(
    const uint64_t& hash,
    const Individual& individual,
    const std::vector<Data::DataView>& inputs,
    const Problem& problem)
{
    if(hash == this->currentHash) {
        if(dynamic_cast<const ReinforcementProblem*>(&problem) == nullptr) {
            throw std::runtime_error ("Evaluation::RewardsMetric::extractBeforeExecution: feature can only be extracted on a reinforcementProblem");
        }
        this->currentScore += dynamic_cast<const ReinforcementProblem&>(problem).getEnvironment().getLastReward();
    } else {
        this->currentHash = hash;
        this->currentScore = 0.0;
    }
}

void Evaluation::RewardsMetric::extractionEndAndComputeMetric(
    const uint64_t& hash,
    const Individual& individual,
    const Problem& problem) 
{
    if(dynamic_cast<const ReinforcementProblem*>(&problem) == nullptr) {
        throw std::runtime_error ("Evaluation::RewardsMetric::extractBeforeExecution: feature can only be extracted on a reinforcementProblem");
    }
    this->currentScore += dynamic_cast<const ReinforcementProblem&>(problem).getEnvironment().getLastReward();

    this->metrics.insert(std::make_pair(hash, Data::DataValue::scalar<double>(this->currentScore)));
}

std::string Evaluation::RewardsMetric::toString(std::string prefix) const
{
    return "todo";
}