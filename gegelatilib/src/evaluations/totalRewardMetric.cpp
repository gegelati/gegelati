#include "evaluations/totalRewardMetric.h"

        
std::map<size_t, std::unique_ptr<Evaluations::Feature>> Evaluations::TotalRewardMetric::getRequestedFeatures() const
{
    std::map<size_t, std::unique_ptr<Evaluations::Feature>> map;
    map.insert(std::make_pair(this->rewardFeatureHash, std::make_unique<RewardFeature>()));
    return std::move(map);
}

Data::DataValue Evaluations::TotalRewardMetric::computeMetrics(const std::map<size_t, std::unique_ptr<Feature>>& features) const 
{
    if(features.find(rewardFeatureHash) == features.end()) {
        throw std::runtime_error("Evaluations::totalRewardMetric::computeMetrics: features does not contains a rewardFeature");
    }
    const Feature& rewardFeature = *features.at(rewardFeatureHash);
    double score = 0.0;
    for(const size_t& key: rewardFeature.getKeys()) {
        size_t nbRewards = rewardFeature.getFeatureAt(key).getDimensions().at(0);
        std::shared_ptr<const double []> rewards = rewardFeature.getFeatureAt(key).getData<double>();
        
        for(size_t idx = 0; idx < nbRewards; idx++) {
            score += rewards[idx];
        }
    }

    // Average over episodes
    score /= rewardFeature.size();

    return Data::DataValue::scalar<double>(score);
}