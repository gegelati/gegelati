/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2020) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2020)
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

#include "learn/adversarialEvaluationResult.h"
#include "learn/evaluationResult.h"

TEST(AdversarialEvaluationResultTest, Constructor)
{
    Learn::AdversarialEvaluationResult* er = NULL;

    ASSERT_NO_THROW(er = new Learn::AdversarialEvaluationResult(1, 5))
        << "Construction of the Adversarial Learning Result failed.";

    ASSERT_EQ(1, er->getSize()) << "The adversarial evaluation result doesn't "
                                   "have the right size after construction.";
    ASSERT_EQ(5, er->getNbEvaluation())
        << "The adversarial evaluation result doesn't have the right number of "
           "evaluations after construction.";

    ASSERT_NO_THROW(delete er)
        << "Destruction of the Adversarial Learning Result failed.";

    ASSERT_NO_THROW(er = new Learn::AdversarialEvaluationResult({1, 2, 6}, 5))
        << "Construction of the Adversarial Learning Result failed.";

    ASSERT_EQ(3, er->getSize()) << "The adversarial evaluation result doesn't "
                                   "have the right size after construction.";
    ASSERT_EQ(3, er->getResult()) // should simply be the first value
        << "The adversarial evaluation result doesn't have the right value "
           "after construction.";
    ASSERT_EQ(1, er->getScoreOf(0))
        << "The adversarial evaluation result doesn't have the right value "
           "after construction.";
    ASSERT_EQ(2, er->getScoreOf(1))
        << "The adversarial evaluation result doesn't have the right value "
           "after construction.";
    ASSERT_EQ(6, er->getScoreOf(2))
        << "The adversarial evaluation result doesn't have the right value "
           "after construction.";
    ASSERT_EQ(5, er->getNbEvaluation())
        << "The adversarial evaluation result doesn't have the right number of "
           "evaluations after construction.";

    ASSERT_NO_THROW(delete er)
        << "Destruction of the Adversarial Learning Result failed.";
}

TEST(AdversarialEvaluationResultTest, operatorPlusEqual)
{
    Learn::AdversarialEvaluationResult er({2, 5, 10}, 10);
    Learn::AdversarialEvaluationResult er2({0.5, 2, 4}, 5);
    Learn::AdversarialEvaluationResult er3({0.5, 2, 4, 5}, 5);
    Learn::EvaluationResult er4(0, 5);

    ASSERT_NO_THROW(er += er2)
        << "Sum of two Adversarial Evaluation Result failed.";
    ASSERT_EQ(3, er.getSize()) << "Adversarial Evaluation Result doesn't have "
                                  "the right size after sum.";
    ASSERT_EQ(15, er.getNbEvaluation())
        << "Adversarial Evaluation Result doesn't have the right number of "
           "evaluations after sum.";
    ASSERT_EQ(1.5, er.getScoreOf(0)) << "Adversarial Evaluation Result doesn't "
                                        "have the right value after sum.";
    ASSERT_EQ(4, er.getScoreOf(1)) << "Adversarial Evaluation Result doesn't "
                                      "have the right value after sum.";
    ASSERT_EQ(8, er.getScoreOf(2)) << "Adversarial Evaluation Result doesn't "
                                      "have the right value after sum.";
    ASSERT_THROW(er += er3, std::runtime_error)
        << "Adding Adversarial Evaluation Results of different sizes should "
           "throw an exception.";
    ASSERT_THROW(er += er4, std::runtime_error)
        << "Adding Adversarial Evaluation Result with simple Evaluation Result "
           "should throw an exception.";
}

TEST(AdversarialEvaluationResultTest, operatorDivideEqual)
{
    Learn::AdversarialEvaluationResult er({4, 8, 10}, 10);

    ASSERT_NO_THROW(er /= 2)
        << "Division of an Adversarial Evaluation Result failed.";
    ASSERT_EQ(3, er.getSize()) << "Adversarial Evaluation Result doesn't have "
                                  "the right size after division.";
    ASSERT_EQ(10, er.getNbEvaluation())
        << "Adversarial Evaluation Result doesn't have the right number of "
           "evaluations after division.";
    ASSERT_EQ(2, er.getScoreOf(0)) << "Adversarial Evaluation Result doesn't "
                                      "have the right value after division.";
    ASSERT_EQ(4, er.getScoreOf(1)) << "Adversarial Evaluation Result doesn't "
                                      "have the right value after division.";
    ASSERT_EQ(5, er.getScoreOf(2)) << "Adversarial Evaluation Result doesn't "
                                      "have the right value after division.";
}
