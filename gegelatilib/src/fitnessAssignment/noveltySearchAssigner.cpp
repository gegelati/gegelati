
#include "fitnessAssignment/noveltySearchAssigner.h"

#include <cmath>
#include <limits>

std::map<uint64_t, std::vector<double>> FitnessAssignment::NoveltySearchAssigner::computeDescriptors(
    const std::map<uint64_t, std::unique_ptr<Metrics::MetricMap>>& individualMetrics) const
{
    // Compute the descriptor values of each individual
    std::map<uint64_t, std::vector<double>> descriptorValues;

    for (const auto& [individualID, metricMap] : individualMetrics){

        std::vector<double> currentDescriptorValues = this->descriptor->compute(*metricMap);
        
        if(currentDescriptorValues.size() == 0) {
            throw std::runtime_error("FitnessAssignment::NoveltySearchAssigner::computeDescriptors should not have zero descriptor values");
        }
        if(descriptorValues.size() > 0 && currentDescriptorValues.size() != descriptorValues.begin()->second.size()) {
            throw std::runtime_error("FitnessAssignment::NoveltySearchAssigner::computeDescriptors descriptors should always have the same size");
        }

        descriptorValues.insert(std::make_pair(individualID, std::move(currentDescriptorValues)));

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

            // A descriptor with a NaN or an infinite value (e.g. the output of a program computing a log or a division) has no meaningful
            // distance: it is infinite, so that it is never a nearest neighbor (and the sort of the neighbors is well defined).
            if(!std::isfinite(distance)) {
                distance = std::numeric_limits<double>::infinity();
            }

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

        // Sort the other individuals by distance from X.
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

        if(neighborsX.empty()) {
            throw std::runtime_error("FitnessAssignment::NoveltySearchAssigner::computeNoveltyScores list of neighbors should not be empty");
        }

        // Infinite distances (see computeDistances) are ignored: they are the last neighbors, taken only when there are not enough finite ones.
        // An individual without any finite distance is the worst one (zero novelty), it must not be the most novel.
        double novelty = 0.0;
        size_t nbFinite = 0;
        for(const uint64_t& indivY : neighborsX) {
            const double distance = distances.at(indivX).at(indivY);
            if(std::isfinite(distance)) {
                novelty += distance;
                nbFinite++;
            }
        }
        novelty = (nbFinite > 0) ? novelty / static_cast<double>(nbFinite) : 0.0;

        noveltyScores.insert(std::make_pair(indivX, novelty));
    }

    return std::move(noveltyScores);
}



std::map<uint64_t, double> FitnessAssignment::NoveltySearchAssigner::assignFitness(
                const std::map<uint64_t, std::unique_ptr<Metrics::MetricMap>>& individualMetrics,
                uint64_t metricHash) const
{
    std::map<uint64_t, std::vector<double>> descriptorValues = this->computeDescriptors(individualMetrics);

    std::map<uint64_t, std::map<uint64_t, double>> distances = this->computeDistances(descriptorValues);

    std::map<uint64_t, std::vector<uint64_t>> neighbors = this->computeNeighbors(distances);

    return this->computeNoveltyScores(distances, neighbors);
}