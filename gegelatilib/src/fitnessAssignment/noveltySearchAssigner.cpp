
#include "fitnessAssignment/noveltySearchAssigner.h"

std::vector<double> FitnessAssignment::NoveltySearchAssigner::computeOneDescriptorAvgVector(
    const std::map<uint64_t, const Metrics::VectorMetric*>& vectorMetrics) const
{
    auto it = vectorMetrics.begin();
    std::vector<double> averageDescriptor = it->second->getValues();
    it++;
    while(it != vectorMetrics.end()) {

        const std::vector<double>& currentValues = it->second->getValues();
        if(currentValues.size() != averageDescriptor.size()) {
            throw std::runtime_error("FitnessAssignment::NoveltySearchAssigner::computeDescriptors: All metrics must have the same number of values.");
        }
            
        for(size_t idx = 0; idx < averageDescriptor.size(); idx++) {
            averageDescriptor.at(idx) += currentValues.at(idx);
        }

        it++;
    }

    for(size_t idx = 0; idx < averageDescriptor.size(); idx++) {
        averageDescriptor.at(idx) /= vectorMetrics.size();
    }
    return std::move(averageDescriptor);
}

std::vector<double> FitnessAssignment::NoveltySearchAssigner::computeOneDescriptorListScalar(
    const std::map<uint64_t, const Metrics::ScalarMetric*>& scalarMetrics) const
{
    std::vector<double> listDescriptors;
    for(const auto& [hash, scalarMetric]: scalarMetrics) {
        listDescriptors.push_back(scalarMetric->getValue());
    }
    return std::move(listDescriptors);
}

std::map<uint64_t, std::vector<double>> FitnessAssignment::NoveltySearchAssigner::computeDescriptors(
    const std::map<std::shared_ptr<const Individual>, std::unique_ptr<Metrics::MetricMap>, SharedLess<Individual>>& individualMetrics,
    uint64_t metricHash) const
{
    // Get the average values of each descriptor for each individual
    std::map<uint64_t, std::vector<double>> descriptorValues;

    for (const auto& [individual, metricMap] : individualMetrics){

        std::vector<double> currentDescriptorValues;
        if(this->averageFeature) {
            currentDescriptorValues = this->computeOneDescriptorAvgVector(metricMap->getMetricValues<Metrics::VectorMetric>(metricHash));
        } else {
            currentDescriptorValues = this->computeOneDescriptorListScalar(metricMap->getMetricValues<Metrics::ScalarMetric>(metricHash));
        }
        
        if(currentDescriptorValues.size() == 0) {
            throw std::runtime_error("FitnessAssignment::NoveltySearchAssigner::computeDescriptors should not have zero descriptor values");
        }
        if(descriptorValues.size() > 0 && currentDescriptorValues.size() != descriptorValues.begin()->second.size()) {
            throw std::runtime_error("FitnessAssignment::NoveltySearchAssigner::computeDescriptors descriptors should always have the same size");
        }

        descriptorValues.insert(std::make_pair(individual->getIndividualID(), std::move(currentDescriptorValues)));

    }

    return std::move(descriptorValues);
}

std::map<uint64_t, std::map<uint64_t, double>> FitnessAssignment::NoveltySearchAssigner::computeDistances(std::map<uint64_t, std::vector<double>> descriptors) const
{
    std::map<uint64_t, std::map<uint64_t, double>> distances;
    for(const auto& [indivX, valuesX]: descriptors) {
        std::map<uint64_t, double> distancesX;
        
        for(const auto& [indivY, valuesY]: descriptors) {
            // Ignore same individuals
            if(indivX == indivY) {
                continue;
            }

            double distance = 0.0;
            for(size_t idx = 0; idx < valuesX.size(); idx++) {
                distance += std::pow(valuesX.at(idx) - valuesY.at(idx), 2);
            }
            distance = std::sqrt(distance);

            distancesX.insert(std::make_pair(indivY, distance));
        }
        distances.insert(std::make_pair(indivX, std::move(distancesX)));
    }
    return std::move(distances);
}

std::map<uint64_t, std::vector<uint64_t>> FitnessAssignment::NoveltySearchAssigner::computeNeighbors(std::map<uint64_t, std::map<uint64_t, double>> distances) const
{
    std::map<uint64_t, std::vector<uint64_t>> neighbors;

    for(const auto& [indivX, distancesX] : distances) {

        // Sort vertices by distance from X.
        std::vector<std::pair<uint64_t, double>> sortedDistances(
            distancesX.begin(), distancesX.end());

        std::sort( sortedDistances.begin(), sortedDistances.end(),
                [](const auto& a, const auto& b) {
                    return a.second < b.second;
        });

        if(this->nbNeighbors > sortedDistances.size()) {
            throw std::runtime_error("FitnessAssignment::NoveltySearchAssigner::computeNeighbors number of neighbors above the size of individuals");
        }

        std::vector<uint64_t> xNeighbors;
        for(size_t i = 0; i < this->nbNeighbors; ++i) {
            xNeighbors.push_back(sortedDistances[i].first);
        }

        neighbors.insert(std::make_pair(indivX, xNeighbors));
    }

    return std::move(neighbors);
}

std::map<uint64_t, double> FitnessAssignment::NoveltySearchAssigner::computeNoveltyScores(
    std::map<uint64_t, std::map<uint64_t, double>> distances,
    std::map<uint64_t, std::vector<uint64_t>> neighbors) const
{
    std::map<uint64_t, double> noveltyScores;

    for(const auto& [indivX, neighborsX] : neighbors) {

        double novelty = 0.0;

        for(const uint64_t& indivY : neighborsX) {
            novelty += distances.at(indivX).at(indivY);
        }

        if(neighborsX.empty()) {
            throw std::runtime_error("FitnessAssignment::NoveltySearchAssigner::computeNoveltyScores list of neighbors should not be empty");
        }
        novelty /= static_cast<double>(neighborsX.size());

        noveltyScores.insert(std::make_pair(indivX, novelty));
    }

    return std::move(noveltyScores);
}



std::map<std::shared_ptr<const Individual>, double, SharedLess<Individual>> FitnessAssignment::NoveltySearchAssigner::assignFitness(
                const std::map<std::shared_ptr<const Individual>, std::unique_ptr<Metrics::MetricMap>, SharedLess<Individual>>& individualMetrics,
                uint64_t metricHash) const
{
    std::map<uint64_t, std::vector<double>> descriptorValues = this->computeDescriptors(individualMetrics, metricHash);

    std::map<uint64_t, std::map<uint64_t, double>> distances = this->computeDistances(descriptorValues);

    std::map<uint64_t, std::vector<uint64_t>> neighbors = this->computeNeighbors(distances);

    std::map<uint64_t, double> noveltyScores = this->computeNoveltyScores(distances, neighbors);

    // Get back to individuals instead of IDs.
    std::map<std::shared_ptr<const Individual>, double, SharedLess<Individual>> fitnesses;
    for(const auto& [indiv, metricMap]: individualMetrics) {
        fitnesses.insert(std::make_pair(indiv, noveltyScores.at(indiv->getIndividualID())));
    }

    return std::move(fitnesses);
}