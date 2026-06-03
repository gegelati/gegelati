/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2025) :
 *
 * Quentin Vacher <qvacher@insa-rennes.fr> (2025)
 *
 * GEGELATI is an open-source reinforcement learning framework for training
 * artificial intelligence based on Tangled Program Graphs (TPGs).
 *
 * This software is governed by the CeCILL-C license under French law and
 * abiding by the rules of distribution of free software. You can use,
 * modify and/ or redistribute the software under the terms of the CeCILL-C
 * license as circulated by CEA, CNRS and INRIA at the following URL
 * "http://www.cecill.info".
 *
 * As a counterpart to the access to the source code and rights to copy,
 * modify and redistribute granted by the license, users are provided only
 * with a limited warranty and the software's author, the holder of the
 * economic rights, and the successive licensors have only limited
 * liability.
 *
 * In this respect, the user's attention is drawn to the risks associated
 * with loading, using, modifying and/or developing or reproducing the
 * software by the user in light of its specific status of free software,
 * that may mean that it is complicated to manipulate, and that also
 * therefore means that it is reserved for developers and experienced
 * professionals having in-depth computer knowledge. Users are therefore
 * encouraged to load and test the software's suitability as regards their
 * requirements in conditions enabling the security of their systems and/or
 * data to be ensured and, more generally, to use and operate it in the
 * same conditions as regards security.
 *
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL-C license and that you accept its terms.
 */



#include <numeric>

#include "learn/classificationLearningEnvironment.h"
#include "selector/classificationSelectionMetrics.h"

void Selector::ClassificationSelectionMetrics::initMetrics(
    const TPG::TPGVertex* agent,
    const Learn::LearningEnvironment& learningEnvironment)
{
    this->scorePerClass.resize(learningEnvironment.getNbActions(), 0.0);
    this->nbEvalPerClass.resize(learningEnvironment.getNbActions(), 0.0);
}

void Selector::ClassificationSelectionMetrics::extractMetricsEpisode(
    const TPG::TPGVertex* agent, size_t nbStepsExecuted,
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
