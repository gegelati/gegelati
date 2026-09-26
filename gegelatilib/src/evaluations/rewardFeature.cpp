#include "evaluations/rewardFeature.h"

void Evaluations::RewardFeature::extractBeforeExecution(
    const uint64_t& hash,
    const Individual& individual,
    const std::vector<Data::DataView>& inputs,
    const Problem& problem)
{
    if(newEpisode) {
        newEpisode = false;
    } else {
        const ReinforcementProblem& rlProblem = dynamic_cast<const ReinforcementProblem&>(problem);
        const ReinforcementEnvironment& rlEnv = dynamic_cast<const ReinforcementEnvironment&>(rlProblem.getEnvironment());

        if(rlEnv.isTerminal()) {
            newEpisode = true;
            this->currentRewards.push_back(rlEnv.getLastReward());
            this->features.insert(std::make_pair(hash, Data::DataValue::array1d(this->currentRewards)));
            this->currentRewards.clear();
        } else {
            this->currentRewards.push_back(rlEnv.getLastReward());
        }
    }
}

std::string Evaluations::RewardFeature::toString(std::string prefix) const
{
    return "todo";
}