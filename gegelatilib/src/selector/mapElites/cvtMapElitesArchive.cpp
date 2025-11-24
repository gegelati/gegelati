
#include <cmath>
#include <iostream>
#include <limits>
#include <numeric>
#include <random>
#include <unordered_map>
#include <vector>

#include "selector/mapElites/cvtMapElitesArchive.h"

double Selector::MapElites::CvtMapElitesArchive::dist_squared(
    const std::vector<double>& a, const std::vector<double>& b)
{
    double sum = 0.0;
    for (size_t i = 0; i < this->nbDescriptors; ++i)
        sum += (a[i] - b[i]) * (a[i] - b[i]);
    return sum;
}

std::vector<double> Selector::MapElites::CvtMapElitesArchive::add(
    const std::vector<double>& a, const std::vector<double>& b)
{
    std::vector<double> res(this->nbDescriptors);
    for (size_t i = 0; i < this->nbDescriptors; ++i)
        res[i] = a[i] + b[i];
    return res;
}

std::vector<double> Selector::MapElites::CvtMapElitesArchive::scalar_mult(
    const std::vector<double>& a, double s)
{
    std::vector<double> res(this->nbDescriptors);
    for (size_t i = 0; i < this->nbDescriptors; ++i)
        res[i] = a[i] * s;
    return res;
}

std::vector<double> Selector::MapElites::CvtMapElitesArchive::average(
    const std::vector<std::vector<double>>& points)
{
    std::vector<double> avg(this->nbDescriptors, 0.0);
    if (points.empty())
        return avg;
    for (const auto& p : points)
        avg = add(avg, p);
    return scalar_mult(avg, 1.0 / points.size());
}

std::vector<double> Selector::MapElites::CvtMapElitesArchive::random_point(
    RNG::RNG& rng)
{
    std::vector<double> point(this->nbDescriptors);
    for (size_t i = 0; i < this->nbDescriptors; ++i)
        point[i] = rng.getDouble(this->minValue, this->maxValue);
    return point;
}

size_t Selector::MapElites::CvtMapElitesArchive::nearest(
    const std::vector<double>& point,
    const std::vector<std::vector<double>>& centroids)
{
    double best_dist = std::numeric_limits<double>::max();
    int64_t best_idx = -1;
    for (size_t i = 0; i < centroids.size(); ++i) {
        double d = dist_squared(point, centroids[i]);
        if (d < best_dist) {
            best_dist = d;
            best_idx = i;
        }
    }
    if (best_idx == -1) {
        throw std::runtime_error("Should have found a best centroid");
    }
    return best_idx;
}

void Selector::MapElites::CvtMapElitesArchive::initialize_cvt(RNG::RNG& rng)
{
    for (auto& c : centroids)
        c = random_point(rng);

    std::vector<size_t> j(this->nbCentroids,
                          1); // Compteur d'updates par centroïde

    for (size_t iter = 0; iter < this->nbIterationInit; ++iter) {

        // print progress with a line overrite at each iteration
        std::vector<std::vector<double>> samples(this->nbDotsInit);
        std::vector<std::vector<std::vector<double>>> assignments(
            this->nbCentroids);

        // Create random dots
        for (size_t i = 0; i < this->nbDotsInit; ++i)
            samples[i] = random_point(rng);

        // Assign each dot to the closest centroid
        for (const auto& p : samples) {
            size_t idx = nearest(p, this->centroids);
            assignments[idx].push_back(p);
        }

        // Update the centroids
        for (size_t i = 0; i < this->nbCentroids; ++i) {
            if (!assignments[i].empty()) {
                std::vector<double> u_i = average(assignments[i]);
                std::vector<double> z_i = centroids[i];
                size_t j_i = j[i];

                // Update
                std::vector<double> new_z(this->nbDescriptors);
                for (size_t d = 0; d < this->nbDescriptors; ++d) {
                    new_z[d] = ((this->a1 * j_i + this->b1) * z_i[d] +
                                (this->a2 * j_i + this->b2) * u_i[d]) /
                               (j_i + 1);
                }

                centroids[i] = new_z;
                j[i]++;
            }
        }
    }
    std::cout << std::endl;
}

const std::pair<std::shared_ptr<Learn::EvaluationResult>,
                std::shared_ptr<const Algorithm::Agent>>&
Selector::MapElites::CvtMapElitesArchive::getArchiveFromDescriptors(
    const std::vector<double>& descriptors) const
{
    size_t idx = getIndexForDescriptor(descriptors);
    return archive[idx];
}

void Selector::MapElites::CvtMapElitesArchive::setArchiveFromDescriptors(
    std::shared_ptr<const Algorithm::Agent> vertex, std::shared_ptr<Learn::EvaluationResult> eval,
    const std::vector<double>& descriptors)
{
    size_t idx = getIndexForDescriptor(descriptors);
    archive[idx] = std::make_pair(eval, vertex);
}

size_t Selector::MapElites::CvtMapElitesArchive::getIndexForDescriptor(
    const std::vector<double>& descriptors) const
{
    size_t bestIndex = 0;
    double bestDist = std::numeric_limits<double>::max();

    for (size_t i = 0; i < centroids.size(); ++i) {
        double dist = 0.0;
        for (size_t d = 0; d < descriptors.size(); ++d) {
            double diff = descriptors[d] - centroids[i][d];
            dist += diff * diff;
        }
        if (dist < bestDist) {
            bestDist = dist;
            bestIndex = i;
        }
    }

    return bestIndex;
}

const std::vector<std::vector<double>>& Selector::MapElites::
    CvtMapElitesArchive::getCentroids() const
{
    return centroids;
}
