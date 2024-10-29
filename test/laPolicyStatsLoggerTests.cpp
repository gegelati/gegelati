/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2020 - 2024) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2020 - 2022)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2020)
 * Quentin Vacher <qvacher@insa-rennes.fr> (2024)
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

#include "instructions/addPrimitiveType.h"
#include "learn/learningAgent.h"
#include "learn/stickGameWithOpponent.h"

#include "log/laPolicyStatsLogger.h"

class LAPolicyStatsLoggerTest : public ::testing::Test
{
  protected:
    Instructions::Set set;
    StickGameWithOpponent le;
    Learn::LearningParameters params;
    Learn::LearningAgent* la;

    void SetUp() override
    {
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
        params.nbProgramConstant = 0;

        params.archiveSize = 50;
        params.archivingProbability = 0.5;
        params.maxNbActionsPerEval = 11;
        params.nbIterationsPerPolicyEvaluation = 3;
        params.ratioDeletedRoots =
            0.95; // high number to force the apparition of root action.
        params.nbThreads = 1;

        set.add(*(new Instructions::AddPrimitiveType<int>()));
        set.add(*(new Instructions::AddPrimitiveType<double>()));

        la = new Learn::LearningAgent(le, set, params);
    }

    void TearDown() override
    {
        delete la;
        delete (&set.getInstruction(0));
        delete (&set.getInstruction(1));
    }
};

TEST_F(LAPolicyStatsLoggerTest, Constructor)
{
    Log::LAPolicyStatsLogger* log = nullptr;
    ASSERT_NO_THROW(log = new Log::LAPolicyStatsLogger(*la));
    if (log != nullptr) {
        delete log;
    }
}

TEST_F(LAPolicyStatsLoggerTest, LogAfterEvaluate)
{
    // Train one generatio before adding the logger.
    uint64_t genNumber = 42;
    la->init(2);
    la->trainOneGeneration(genNumber);

    // add the Logger
    std::stringstream strStr;
    Log::LAPolicyStatsLogger log(*la, strStr);

    ASSERT_NO_THROW(log.logNewGeneration(genNumber))
        << "This call should not throw any exception.";
    // No need to give anything as a parameter.
    std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                  const TPG::TPGVertex*>
        emptyMap;
    ASSERT_NO_THROW(log.logAfterDecimate())
        << "Logging after an evaluation failed unexpectedly.";

    ASSERT_GT(strStr.str().size(), 100)
        << "String logged by the LAPolicyStatsLogger should be long.";

    auto length = strStr.str().size();

    ASSERT_NO_THROW(log.logAfterDecimate())
        << "Logging a second time after an evaluation failed unexpectedly.";

    ASSERT_EQ(strStr.str().size(), length)
        << "Second call to logAfterEvaluate should not log anything new, the "
           "bestRoot not having been replaced.";

    // Train a new gen (calls the log)
    ASSERT_NO_THROW(la->trainOneGeneration(
        genNumber + 1)) // +1 deterministically creates a new bestRoot.
        << "Training a new generation should not cause any problem.";

    ASSERT_GT(strStr.str().size(), length)
        << "Training a new generation (which deterministically creates a new "
           "bestRoot) should result in new log being written.";
}

TEST_F(LAPolicyStatsLoggerTest, EmptyMethods)
{
    std::stringstream strStr;
    Log::LAPolicyStatsLogger log(*la, strStr);

    // Explicit calls to empty method to force code coverage.
    // These methods are called during la.trainOneGeneration
    std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                  const TPG::TPGVertex*>
        emptyMap;
    log.logAfterPopulateTPG();
    log.logEndOfTraining();
    log.logAfterEvaluate(emptyMap);

    // These methods are not called otherwise
    log.logHeader();
    log.logAfterValidate(emptyMap);

    ASSERT_EQ(strStr.str().size(), 0)
        << "Empty method should not generate any log.";
}
