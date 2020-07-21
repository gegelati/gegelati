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
    la->init();
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
        genNumber + 2)) // +2 deterministically creares a new bestRoot.
        << "Training a new generation should not cause any problem.";

    ASSERT_GT(strStr.str().size(), length)
        << "Training a new generation (which deterministically creates a new "
           "bestRoot) should result in new log being written.";
}