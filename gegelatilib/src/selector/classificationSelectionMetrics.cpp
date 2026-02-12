

#include <numeric>

#include "learn/classificationLearningEnvironment.h"
#include "selector/classificationSelectionMetrics.h"

void Selector::ClassificationSelectionMetrics::initMetrics(
    std::shared_ptr<const Algorithm::Agent> agent,
    const Learn::LearningEnvironment& learningEnvironment)
{
    this->scorePerClass.resize(
        learningEnvironment.getActions()->front().getNbValues(), 0.0);
    this->nbEvalPerClass.resize(
        learningEnvironment.getActions()->front().getNbValues(), 0.0);
}

void Selector::ClassificationSelectionMetrics::extractMetricsEpisode(
    std::shared_ptr<const Algorithm::Agent> agent, size_t nbStepsExecuted,
    const Learn::LearningEnvironment& learningEnvironment)
{
    // Update results
    const auto& classificationTable =
        ((Learn::ClassificationLearningEnvironment&)learningEnvironment)
            .getClassificationTable();
    // for each class
    for (uint64_t classIdx = 0; classIdx < classificationTable.size();
         classIdx++) {
        uint64_t truePositive = classificationTable.at(classIdx).at(classIdx);
        uint64_t falseNegative =
            std::accumulate(classificationTable.at(classIdx).begin(),
                            classificationTable.at(classIdx).end(),
                            (uint64_t)0) -
            truePositive;
        uint64_t falsePositive = 0;
        std::for_each(classificationTable.begin(), classificationTable.end(),
                      [&classIdx, &falsePositive](
                          const std::vector<uint64_t>& classifForClass) {
                          falsePositive += classifForClass.at(classIdx);
                      });
        falsePositive -= truePositive;

        double recall =
            (double)truePositive / (double)(truePositive + falseNegative);
        double precision =
            (double)truePositive / (double)(truePositive + falsePositive);
        // If true positive is 0, set score to 0.
        double fScore = (truePositive != 0)
                            ? 2 * (precision * recall) / (precision + recall)
                            : 0.0;
        scorePerClass.at(classIdx) += fScore;

        nbEvalPerClass.at(classIdx) += truePositive + falseNegative;
    }
}

const std::vector<double>& Selector::ClassificationSelectionMetrics::
    getScorePerClass() const
{
    return this->scorePerClass;
}

const std::vector<size_t>& Selector::ClassificationSelectionMetrics::
    getNbEvalPerClassPerClass() const
{
    return this->nbEvalPerClass;
}

void Selector::ClassificationSelectionMetrics::weightedSum(
    std::shared_ptr<SelectionMetrics> other, size_t nbEvaluation,
    size_t nbEvaluationOther)
{

    // To update main score and utility, and check type issues
    SelectionMetrics::weightedSum(other, nbEvaluation, nbEvaluationOther);

    const auto& castedOther =
        static_cast<const Selector::ClassificationSelectionMetrics&>(*other);
    if (this->scorePerClass.size() != castedOther.scorePerClass.size()) {
        throw std::runtime_error("Number of classes is not the same.");
    }

    for (size_t idx = 0; idx < this->scorePerClass.size(); idx++) {
        // Weighted sum of the score per class.
        this->scorePerClass[idx] =
            this->scorePerClass[idx] * (double)this->nbEvalPerClass[idx] +
            castedOther.scorePerClass[idx] *
                (double)castedOther.nbEvalPerClass[idx];
        this->nbEvalPerClass[idx] += castedOther.nbEvalPerClass[idx];

        this->scorePerClass[idx] /= (double)this->nbEvalPerClass[idx];
    }
}