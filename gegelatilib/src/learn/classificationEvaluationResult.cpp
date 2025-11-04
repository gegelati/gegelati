/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2020) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2020)
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
#include <stdexcept>

#include "learn/classificationEvaluationResult.h"

std::shared_ptr<Selector::SelectionMetrics> Learn::ClassificationEvaluationResult::computeAverageMetrics(const std::vector<std::shared_ptr<Selector::SelectionMetrics>>& selectionMetricsPerClass)
{
    // Aggregated result kept in a static local to return a reference without
    // allocating a new shared_ptr. Reset it at each call.
    Selector::SelectionMetrics average;

    /*for (const auto& metric : selectionMetricsPerClass) {
        average += (*metric);
    }

    average /= static_cast<double>(selectionMetricsPerClass.size());*/

    return std::make_shared<Selector::SelectionMetrics>(average);
}

const std::vector<std::shared_ptr<Selector::SelectionMetrics>>& Learn::ClassificationEvaluationResult::getSelectionMetricsPerClassPerClass() const
{
    return this->selectionMetricsPerClass;
}

const std::vector<size_t>& Learn::ClassificationEvaluationResult::
    getNbEvaluationPerClass() const
{
    return this->nbEvaluationPerClass;
}

Learn::EvaluationResult& Learn::ClassificationEvaluationResult::operator+=(
    const EvaluationResult& other)
{
    // Super call to detect type mismatch.
    /*EvaluationResult::operator+=(other);

    // If types are identical, add per-class metrics
    if (typeid(*this) == typeid(other)) {
        const ClassificationEvaluationResult& otherEval =
            static_cast<const ClassificationEvaluationResult&>(other);
        if (this->selectionMetricsPerClass.size() != otherEval.selectionMetricsPerClass.size()) {
            throw std::runtime_error(
                "Number of metrics per class is different between the added "
                "ClassificationEvaluationResult.");
        }

        for (auto idx = 0; idx < this->selectionMetricsPerClass.size(); idx++) {
            auto& curMetrics = this->selectionMetricsPerClass.at(idx);
            const auto& otherMetrics = otherEval.selectionMetricsPerClass.at(idx);

            // Scale current metrics by its weight
            (*curMetrics) *= (double)this->nbEvaluationPerClass.at(idx);

            // Make a temporary copy of other metrics
            Selector::SelectionMetrics tempMetrics(*otherMetrics);
            tempMetrics *= (double)otherEval.nbEvaluationPerClass.at(idx);

            // Add weighted metrics
            (*curMetrics) += tempMetrics;

            // Normalize by total evaluations
            (*curMetrics) /= (double)(this->nbEvaluationPerClass.at(idx) + 
                                    otherEval.nbEvaluationPerClass.at(idx));

            // Sum number of evaluations per class
            this->nbEvaluationPerClass.at(idx) +=
                otherEval.nbEvaluationPerClass.at(idx);
        }

        // Update global number of evaluations
        this->nbEvaluation += otherEval.nbEvaluation;

        // Update the global selectionMetrics by computing average across all classes
        this->selectionMetrics = computeAverageMetrics(this->selectionMetricsPerClass);
    }*/

    return *this;
}
