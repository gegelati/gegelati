#include "evaluations/metric.h"

std::map<size_t, std::unique_ptr<Evaluations::Feature>> Evaluations::Metric::getUniqueRequestedMetrics(
    const std::vector<std::shared_ptr<const Metric>>& metrics)
{
    if(metrics.empty()) {
        throw std::runtime_error(
            "Evaluations::Metric::getUniqueRequestedMetrics: "
            "cannot evaluate with empty list of metrics");
    }

    std::map<size_t, std::unique_ptr<Feature>> features =
        metrics.at(0)->getRequestedFeatures();


    // Add external features only if their hash is unseen
    for(size_t idx = 1; idx < metrics.size(); ++idx) {
        std::map<size_t, std::unique_ptr<Feature>> otherFeatures =
            metrics.at(idx)->getRequestedFeatures();

        for(auto& [key, feature] : otherFeatures) {
            features.emplace(key, std::move(feature));
        }
    }

    return features;
}