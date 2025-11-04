/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2020) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2020)
 * Pierre-Yves Le Rolland-Raumer <plerolla@insa-rennes.fr> (2020)
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

#include "learn/adversarialEvaluationResult.h"

std::shared_ptr<Selector::SelectionMetrics> Learn::AdversarialEvaluationResult::getSelectionMetricsOf(int index)
{
    return this->multiSelectionMetrics.at(index);
}

Learn::EvaluationResult& Learn::AdversarialEvaluationResult::operator+=(
    const EvaluationResult& other)
{
    // Type Check (Must be done in all override)
    // This test will succeed in child class.
    const std::type_info& thisType = typeid(*this);
    if (typeid(other) != thisType) {
        throw std::runtime_error("Type mismatch between EvaluationResults.");
    }

    const AdversarialEvaluationResult& otherConverted =
        static_cast<const AdversarialEvaluationResult&>(other);

    // Size Check
    if (otherConverted.multiSelectionMetrics.size() != this->multiSelectionMetrics.size()) {
        throw std::runtime_error(
            "Size mismatch between AdversarialEvaluationResults.");
    }

    // Weighted addition of results
    const double totalEval = (double)(this->nbEvaluation + otherConverted.nbEvaluation);
    for (size_t i = 0; i < multiSelectionMetrics.size(); i++) {
        auto& curPtr = this->multiSelectionMetrics[i];
        const auto& otherPtr = otherConverted.multiSelectionMetrics[i];

        if (!curPtr) {
            throw std::runtime_error("Null SelectionMetrics pointer encountered 2.");
        }
        if (!curPtr || !otherPtr) {
            throw std::runtime_error("Null SelectionMetrics pointer encountered.");
        }

        // Scale current in-place by its weight
        (*curPtr) *= (double)this->nbEvaluation;

        // Make a temporary copy of other (stack copy, avoids modifying other)
        Selector::SelectionMetrics tempOther(*otherPtr);
        tempOther *= (double)otherConverted.nbEvaluation;

        // Add and normalize, all in-place on current metric
        (*curPtr) += tempOther;
        (*curPtr) /= totalEval;
    }

    // Addition of nbEvaluation
    this->nbEvaluation += otherConverted.nbEvaluation;

    return *this;
}

Learn::EvaluationResult& Learn::AdversarialEvaluationResult::operator/=(
    double divisor)
{
    for (auto& ptr : multiSelectionMetrics) {
        if (!ptr) {
            throw std::runtime_error("Null SelectionMetrics pointer encountered.");
        }
        (*ptr) /= divisor;
    }
    return *this;
}

std::shared_ptr<Selector::SelectionMetrics> Learn::AdversarialEvaluationResult::getSelectionMetrics() const
{
    // Aggregated result kept in a static local to return a reference without
    // allocating a new shared_ptr. Reset it at each call.
    Selector::SelectionMetrics average;

    for (const auto& metric : multiSelectionMetrics) {
        average += (*metric);
    }

    average /= static_cast<double>(getSize());

    return std::make_shared<Selector::SelectionMetrics>(average);
}

size_t Learn::AdversarialEvaluationResult::getSize() const
{
    return multiSelectionMetrics.size();
}