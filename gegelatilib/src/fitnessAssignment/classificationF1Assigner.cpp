
#include "fitnessAssignment/classificationF1Assigner.h"


std::map<uint64_t, double> FitnessAssignment::ClassificationF1Assigner::assignFitness(
    const std::map<uint64_t, std::unique_ptr<Metrics::MetricMap>>& individualMetrics) const
{
    std::map<uint64_t, double> fitnesses;
    size_t numClasses = 0;

    // Determine the number of classes across all individuals.
    for (const auto& [individualID, metricMap] : individualMetrics) {
        const auto predictionMetrics = metricMap->getMetricValues<Metrics::PredictionMetric>(this->metricHash);
        for (const auto& [featureHash, metric] : predictionMetrics) {
            for (const auto& [predictionValue, targetValue] : metric->getPairs()) {
                const size_t prediction = predictionValue.getScalar<size_t>();
                const size_t target = targetValue.getScalar<size_t>();
                numClasses = std::max(numClasses, std::max(prediction, target) + 1);
            }
        }
    }

    if (numClasses == 0) {
        for (const auto& [individualID, metricMap] : individualMetrics) {
            fitnesses[individualID] = 0.0;
        }
        return fitnesses;
    }

    // Compute the macro-averaged F1 score for each individual.
    for (const auto& [individualID, metricMap] : individualMetrics) {
        const auto predictionMetrics = metricMap->getMetricValues<Metrics::PredictionMetric>(this->metricHash);
        std::vector<std::vector<uint64_t>> confusionMatrix(numClasses, std::vector<uint64_t>(numClasses, 0));

        for (const auto& [featureHash, metric] : predictionMetrics) {
            for (const auto& [predictionValue, targetValue] : metric->getPairs()) {
                const size_t prediction = predictionValue.getScalar<size_t>();
                const size_t target = targetValue.getScalar<size_t>();
                ++confusionMatrix.at(target).at(prediction);
            }
        }

        double averageF1Score = 0.0;

        for (size_t classIdx = 0; classIdx < numClasses; ++classIdx) {
            const uint64_t truePositive = confusionMatrix[classIdx][classIdx];
            uint64_t falseNegative = 0;
            for (const uint64_t count : confusionMatrix[classIdx]) {
                falseNegative += count;
            }
            falseNegative -= truePositive;

            uint64_t falsePositive = 0;
            for (size_t actualClass = 0; actualClass < numClasses; ++actualClass) {
                falsePositive += confusionMatrix[actualClass][classIdx];
            }
            falsePositive -= truePositive;

            double recall =
                (double)truePositive / (double)(truePositive + falseNegative);
            double precision =
                (double)truePositive / (double)(truePositive + falsePositive);
            // If true positive is 0, set score to 0.
            double fScore = (truePositive != 0)
                                ? 2 * (precision * recall) / (precision + recall)
                                : 0.0;
            averageF1Score += fScore;
        }

        averageF1Score /= static_cast<double>(numClasses);
        fitnesses[individualID] = averageF1Score;
    }

    return fitnesses;
}