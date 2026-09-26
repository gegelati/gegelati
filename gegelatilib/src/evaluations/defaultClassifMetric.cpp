#include "evaluations/defaultClassifMetric.h"

        
std::map<size_t, std::unique_ptr<Evaluations::Feature>> Evaluations::DefaultClassifMetric::getRequestedFeatures() const
{
    std::map<size_t, std::unique_ptr<Evaluations::Feature>> map;
    map.insert(std::make_pair(this->predictionFeatureHash, std::make_unique<PredictionFeature>()));
    return std::move(map);
}

Data::DataValue Evaluations::DefaultClassifMetric::computeMetrics(const std::map<size_t, std::unique_ptr<Feature>>& features) const 
{
    if(features.find(predictionFeatureHash) == features.end()) {
        throw std::runtime_error("Evaluations::totalPredictionMetric::computeMetrics: features does not contains a predictionFeature");
    }
    const Feature& predictionFeature = *features.at(predictionFeatureHash);
    double ratio = 0.0;
    for(const size_t& key: predictionFeature.getKeys()) {
        ratio += (double)predictionFeature.getFeatureAt(key).getScalar<bool>();
    }

    // Average over episodes
    ratio /= predictionFeature.size();

    return Data::DataValue::scalar<double>(ratio);
}