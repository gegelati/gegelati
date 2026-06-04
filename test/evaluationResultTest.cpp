/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2020 - 2025) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2020)
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

#include <gtest/gtest.h>
#include <stdexcept>

#include "selector/classificationSelectionMetrics.h"

#include "learn/evaluationResult.h"

TEST(EvaluationResultTest, Constructor)
{
    Learn::EvaluationResult* eval;

    ASSERT_NO_THROW(eval = new Learn::EvaluationResult(
                        std::make_shared<Selector::SelectionMetrics>(1.0), 50))
        << "Building an EvaluationResult failed unexpectedly.";

    ASSERT_NO_THROW(delete eval);
}

TEST(EvaluationResultTest, GetResult)
{
    Learn::EvaluationResult eval(
        std::make_shared<Selector::SelectionMetrics>(1.0), 10);

    ASSERT_EQ(eval.getSelectionMetrics()->getScore(), 1.0)
        << "Getter returned an unexpected value.";
}

TEST(EvaluationResultTest, GetNbEvaluation)
{
    Learn::EvaluationResult eval(
        std::make_shared<Selector::SelectionMetrics>(1.0), 10);

    ASSERT_EQ(eval.getNbEvaluation(), 10)
        << "Getter returned an unexpected value.";
}

TEST(EvaluationResultTest, AssignmentAdditionOperator)
{
    Learn::EvaluationResult eval1(
        std::make_shared<Selector::SelectionMetrics>(1.0), 10);
    Learn::EvaluationResult eval2(
        std::make_shared<Selector::SelectionMetrics>(2.0), 20);

    ASSERT_NO_THROW(eval1 += eval2)
        << "Call to operator+= failed unexpectedly.";

    ASSERT_EQ(eval1.getSelectionMetrics()->getScore(),
              (10 * 1.0 + 20 * 2.0) / (10.0 + 20.0))
        << "Getter returned an unexpected value after call to operator+=.";
    ASSERT_EQ(eval2.getSelectionMetrics()->getScore(), 2.0)
        << "Getter returned an unexpected value after call to operator+=.";
    ASSERT_EQ(eval1.getNbEvaluation(), 20 + 10)
        << "Getter returned an unexpected value after call to operator+=.";
    ASSERT_EQ(eval2.getNbEvaluation(), 20)
        << "Getter returned an unexpected value after call to operator+=.";

    Selector::ClassificationSelectionMetrics metric =
        Selector::ClassificationSelectionMetrics({3.0, 4.0}, {2, 3});
    Learn::EvaluationResult eval3(
        std::make_shared<Selector::ClassificationSelectionMetrics>(metric), 0);
    ASSERT_THROW(eval1 += eval3, std::runtime_error)
        << "Call to operator += should not work with heterogeneous "
           "EvaluationResult classes.";
}
