
#include "fitnessAssignment/noveltySearchAssigner.h"

std::map<uint64_t, std::vector<double>> FitnessAssignment::NoveltySearchAssigner::computeDescriptors(
    const std::map<std::shared_ptr<const Individual>, std::unique_ptr<Metrics::MetricMap>, SharedLess<Individual>>& individualMetrics,
    uint64_t metricHash) const
{
    // Get the average values of each descriptor for each individual
    std::map<uint64_t, std::vector<double>> descriptorValues;

    // Number of values should be the same for every individual and every metric of the map.
    size_t expectedNbValues = 0;

    for (const auto& [individual, metricMap] : individualMetrics){

        std::map<uint64_t, const Metrics::VectorMetric*> scalarMetrics = metricMap->getMetricValues<Metrics::VectorMetric>(metricHash);
        if(expectedNbValues == 0) {
            // Get the number of expected values
            expectedNbValues = scalarMetrics.begin()->second->getValues().size();

            // If it was set to 0, throw
            if(expectedNbValues == 0) {
                throw std::runtime_error("FitnessAssignment::NoveltySearchAssigner::computeDescriptors: requires at least one descriptor value.");
            }
        }


        std::vector<double> averageDescriptor(expectedNbValues, 0.0);
        for(const auto& [key, metric]: scalarMetrics) {

            const std::vector<double>& currentValues = metric->getValues();
            if(currentValues.size() != expectedNbValues) {
                throw std::runtime_error("FitnessAssignment::NoveltySearchAssigner::computeDescriptors: All metrics must have the same number of values.");
            }
             
            for(size_t idx = 0; idx < expectedNbValues; idx++) {
                averageDescriptor.at(idx) += currentValues.at(idx);
            }
        }

        for(size_t idx = 0; idx < expectedNbValues; idx++) {
            averageDescriptor.at(idx) /= scalarMetrics.size();
        }
        
        descriptorValues.insert(std::make_pair(individual->getIndividualID(), std::move(averageDescriptor)));
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