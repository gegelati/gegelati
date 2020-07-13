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

#include <gegelati.h>
#include <gtest/gtest.h>
#include <learn/adversarialLearningEnvironment.h>

#include "learn/fakeClassificationLearningEnvironment.h"
#include "learn/learningAgent.h"
#include "learn/learningEnvironment.h"
#include "learn/stickGameAdversarial.h"

TEST(AdversarialLearningEnvironmentTest, Constructor)
{
    Learn::AdversarialLearningEnvironment* le = NULL;

    ASSERT_NO_THROW(le = new StickGameAdversarial())
        << "Construction of the Adversarial Learning Environment failed";

    ASSERT_NO_THROW(delete le)
        << "Destruction of the Adversarial Learning Environment failed";
}

TEST(AdversarialLearningEnvironmentTest, compatibilityLearningAgent)
{
    Learn::AdversarialLearningEnvironment* le = new StickGameAdversarial();

    Instructions::Set set;
    Learn::LearningParameters params;

    set.add(*(new Instructions::AddPrimitiveType<int>()));
    set.add(*(new Instructions::AddPrimitiveType<double>()));

    // Proba as in Kelly's paper
    params.mutation.tpg.maxInitOutgoingEdges = 3;
    params.mutation.prog.maxProgramSize = 96;
    params.mutation.tpg.nbRoots = 15;
    params.mutation.tpg.pEdgeDeletion = 0.7;
    params.mutation.tpg.pEdgeAddition = 0.7;
    params.mutation.tpg.pProgramMutation = 0.2;
    params.mutation.tpg.pEdgeDestinationChange = 0.1;
    params.mutation.tpg.pEdgeDestinationIsAction = 0.5;
    params.mutation.tpg.maxOutgoingEdges = 4;
    params.mutation.prog.pAdd = 0.5;
    params.mutation.prog.pDelete = 0.5;
    params.mutation.prog.pMutate = 1.0;
    params.mutation.prog.pSwap = 1.0;
    params.archiveSize = 50;
    params.archivingProbability = 1.0;
    params.nbIterationsPerPolicyEvaluation = 10;
    params.maxNbActionsPerEval = 11;

    Learn::LearningAgent la(*le, set, params);

    la.init();

    std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                  const TPG::TPGVertex*>
        result;
    ASSERT_NO_THROW(result =
                        la.evaluateAllRoots(0, Learn::LearningMode::TRAINING))
        << "Evaluation from a root failed.";
    ASSERT_EQ(result.size(), la.getTPGGraph().getNbRootVertices())
        << "Number of evaluated roots is under the number of roots from the "
           "TPGGraph.";
}

TEST(AdversarialLearningEnvironmentTest, getScoresAndgetScore)
{
    Learn::AdversarialLearningEnvironment* le = new StickGameAdversarial();

    // makes the first player win
    le->doAction(2);
    le->doAction(2);
    le->doAction(2);
    le->doAction(2);
    le->doAction(2);
    le->doAction(2);
    le->doAction(0);
    le->doAction(1);

    ASSERT_EQ(1.0, le->getScores()->getScoreOf(0));
    ASSERT_EQ(0.0, le->getScores()->getScoreOf(1));
    ASSERT_DOUBLE_EQ(le->getScore(), le->getScores()->getScoreOf(0));

    le->reset();
    // now makes the first player loose with a forbidden move to check the order
    // of the result
    // makes the first player win
    le->doAction(2);
    le->doAction(2);
    le->doAction(2);
    le->doAction(2);
    le->doAction(2);
    le->doAction(2);
    le->doAction(0);
    le->doAction(0);
    le->doAction(2);

    ASSERT_EQ(-1.0, le->getScores()->getScoreOf(0));
    ASSERT_EQ(1.0, le->getScores()->getScoreOf(1));
    ASSERT_DOUBLE_EQ(le->getScore(), le->getScores()->getScoreOf(0));
}
