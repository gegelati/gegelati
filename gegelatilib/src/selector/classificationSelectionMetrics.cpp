

#include <numeric>

#include "selector/classificationSelectionMetrics.h"
#include "learn/classificationLearningEnvironment.h"

void Selector::ClassificationSelectionMetrics::extractMetricsEpisode(const TPG::TPGVertex* agent, const Learn::LearningEnvironment& learningEnvironment)
{
    // Update results
    const auto& classificationTable =
        ((Learn::ClassificationLearningEnvironment&)learningEnvironment)
            .getClassificationTable();
    // for each class
    for (uint64_t classIdx = 0; classIdx < classificationTable.size();
            classIdx++) {
        uint64_t truePositive =
            classificationTable.at(classIdx).at(classIdx);
        uint64_t falseNegative =
            std::accumulate(classificationTable.at(classIdx).begin(),
                            classificationTable.at(classIdx).end(),
                            (uint64_t)0) -
            truePositive;
        uint64_t falsePositive = 0;
        std::for_each(
            classificationTable.begin(), classificationTable.end(),
            [&classIdx, &falsePositive](
                const std::vector<uint64_t>& classifForClass) {
                falsePositive += classifForClass.at(classIdx);
            });
        falsePositive -= truePositive;

        double recall = (double)truePositive /
                        (double)(truePositive + falseNegative);
        double precision = (double)truePositive /
                            (double)(truePositive + falsePositive);
        // If true positive is 0, set score to 0.
        double fScore = (truePositive != 0) ? 2 * (precision * recall) /
                                                    (precision + recall)
                                            : 0.0;
        scorePerClass.at(classIdx) += fScore;

        nbEvalPerClass.at(classIdx) += truePositive + falseNegative;
    }
}

const std::vector<double>& Selector::ClassificationSelectionMetrics::getScorePerClass() const
{
    return this->scorePerClass;
}

const std::vector<size_t>& Selector::ClassificationSelectionMetrics::getNbEvalPerClassPerClass() const
{
    return this->nbEvalPerClass;
}

Selector::SelectionMetrics& Selector::ClassificationSelectionMetrics::operator+=(
    const Selector::SelectionMetrics& other)
{
    // Type Check (Must be done in all override)
    // This test will succeed in child class.
    const std::type_info& thisType = typeid(*this);
    if (typeid(other) != thisType) {
        throw std::runtime_error("Type mismatch between SelectionMetrics.");
    }

    const auto& castedOther = static_cast<const Selector::ClassificationSelectionMetrics&>(other);
    // If the added type is Selector::SelectionMetrics
    if (thisType == typeid(Selector::ClassificationSelectionMetrics)) {
        this->score += castedOther.score;
        this->utility += castedOther.utility;

        if(this->scorePerClass.size() != castedOther.scorePerClass.size()){
            throw std::runtime_error("Number of classes is not the same.");
        }

        for(size_t idx = 0; idx < this->scorePerClass.size(); idx++){
            this->scorePerClass[idx] += castedOther.scorePerClass[idx];
        }

        
        if(this->nbEvalPerClass.size() != castedOther.nbEvalPerClass.size()){
            throw std::runtime_error("Number of classes is not the same.");
        }

        for(size_t idx = 0; idx < this->nbEvalPerClass.size(); idx++){
            this->nbEvalPerClass[idx] += castedOther.nbEvalPerClass[idx];
        }
    }

    return *this;
}


Selector::SelectionMetrics& Selector::ClassificationSelectionMetrics::operator*=(double factor)
{
    this->score *= factor;
    this->utility *= factor;

    for(double& scoreClass: this->scorePerClass){
        scoreClass *= factor;
    }
    return *this;
}